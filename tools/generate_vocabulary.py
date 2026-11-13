#!/usr/bin/env python3
"""Generate the HydroCouple vocabulary (OWL + SKOS) from the interface headers.

The C++ headers are the standard; this script reads them and writes the same
concepts as a vocabulary that other tools can load without a C++ parser:

* every interface (and the plain aggregates BufferDescriptor, ErrorEntry,
  DifferentialEntry, ITransport::Endpoint) becomes an ``owl:Class``, with
  ``rdfs:subClassOf`` from its C++ bases;
* every const, parameterless pure virtual of an interface, and every field of
  an aggregate, becomes a property whose domain is that class and whose range
  is the interface, enumeration or XML Schema type it returns;
* every enumeration becomes both an ``owl:Class`` and a ``skos:ConceptScheme``
  whose members are ``skos:Concept`` individuals carrying their numeric value
  as ``skos:notation``;
* the two lifecycle state machines become ``hc:mayTransitionTo`` links between
  status concepts;
* a class's canonical dimension ordering, stated in prose in its
  documentation, is carried as ``hc:canonicalDimensionOrdering``.

Outputs (under docs/vocabulary/): ``hydrocouple.ttl`` (Turtle),
``hydrocouple.jsonld`` (JSON-LD) and ``index.html`` (one anchor per term, so
every term IRI resolves on the documentation site).

Usage::

    python tools/generate_vocabulary.py           # write the files
    python tools/generate_vocabulary.py --check   # exit 1 if they are stale

hydrocouplehelpers.h (non-normative conveniences), hydrocouplespatialwkb.h (a
byte layout, not concepts) and hydrocouplecomponentabi.h (macros) are not
read. The transition tables come from the Python mirror, which
python/tests/test_transition_parity.py holds equal to the C++ tables.
"""

from __future__ import annotations

import argparse
import html
import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
INCLUDE = ROOT / "include"
OUTPUT = ROOT / "docs" / "vocabulary"

HEADERS = (
    "hydrocouple.h",
    "hydrocoupletemporal.h",
    "hydrocouplespatial.h",
    "hydrocouplespatiotemporal.h",
    "hydrocoupledistributed.h",
)

ONTOLOGY = "https://hydrocouple.org/HydroCouple/vocabulary/"
NS = ONTOLOGY + "#"
PREFIXES = {
    "hc": NS,
    "owl": "http://www.w3.org/2002/07/owl#",
    "rdf": "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
    "rdfs": "http://www.w3.org/2000/01/rdf-schema#",
    "skos": "http://www.w3.org/2004/02/skos/core#",
    "xsd": "http://www.w3.org/2001/XMLSchema#",
    "dcterms": "http://purl.org/dc/terms/",
}

#: C++ value types and the XML Schema datatype each stands for.
PRIMITIVES = {
    "std::string": "xsd:string",
    "string": "xsd:string",
    "double": "xsd:double",
    "float": "xsd:float",
    "bool": "xsd:boolean",
    "int": "xsd:int",
    "int8_t": "xsd:byte",
    "uint8_t": "xsd:unsignedByte",
    "unsigned char": "xsd:unsignedByte",
    "int16_t": "xsd:short",
    "uint16_t": "xsd:unsignedShort",
    "int32_t": "xsd:int",
    "uint32_t": "xsd:unsignedInt",
    "unsigned int": "xsd:unsignedInt",
    "int64_t": "xsd:long",
    "uint64_t": "xsd:unsignedLong",
}


# ── Reading the headers ───────────────────────────────────────────────────


@dataclass
class Comment:
    start: int
    end: int
    text: str

    @property
    def trailing(self) -> bool:  # //!< documents what precedes it on its line
        return self.text.startswith(("//!<", "///<"))

    @property
    def documentation(self) -> bool:
        return self.text.startswith(("/*!", "/**", "//!", "///"))


def mask(text: str) -> tuple[str, list[Comment]]:
    """Blank comments and string contents (offsets and newlines kept)."""
    out = list(text)
    comments: list[Comment] = []
    i, n = 0, len(text)

    def blank(a: int, b: int) -> None:
        for k in range(a, b):
            if out[k] != "\n":
                out[k] = " "

    while i < n:
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            comments.append(Comment(i, j, text[i:j]))
            blank(i, j)
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            comments.append(Comment(i, j, text[i:j]))
            blank(i, j)
            i = j
        elif text[i] == '"':
            j = i + 1
            while j < n and text[j] != '"':
                j += 2 if text[j] == "\\" else 1
            blank(i + 1, j)
            i = j + 1
        else:
            i += 1
    return "".join(out), comments


def clean(comment: str) -> str:
    """Comment text without its markers, one line per source line."""
    lines = []
    for raw in comment.splitlines():
        line = raw.strip()
        line = re.sub(r"^(/\*[!*]?|//[!/]<?|//)", "", line)
        line = re.sub(r"\*/$", "", line)
        line = re.sub(r"^\*(?!/)", "", line)
        lines.append(line.strip())
    return "\n".join(lines).strip()


def inline(text: str) -> str:
    """Doxygen/HTML inline markup reduced to plain text."""
    text = re.sub(r"\\(ref|p|c|b|e|a)\s+", "", text)
    text = re.sub(r"<sup>(.*?)</sup>", r"^\1", text)
    text = re.sub(r"<[^>]+>", "", text)
    return re.sub(r"\s+", " ", text).strip()


def brief(comment: str) -> str:
    """The \\brief paragraph, or the first paragraph when there is none."""
    text = clean(comment)
    match = re.search(r"\\brief\s+", text)
    text = text[match.end():] if match else text
    paragraph = []
    for line in text.splitlines():
        if not line or (line.startswith("\\") and paragraph):
            break
        if line.startswith("\\"):  # a section command before any text
            continue
        paragraph.append(line)
    if not paragraph:  # documented only by what it returns
        returns = re.search(r"\\returns?\s+(.+?)(?:\n\s*\n|\n\\|$)", text, re.DOTALL)
        if returns:
            return "Returns " + inline(returns.group(1))
    return inline(" ".join(paragraph))


def ordering(comment: str) -> str | None:
    """The canonical-dimension-ordering sentence of a comment, if any."""
    text = inline(clean(comment).replace("\n", " "))
    match = re.search(r"Canonical dimension ordering:\s*(.+?\.)(\s|$)", text)
    return match.group(1) if match else None


@dataclass
class Member:
    name: str
    type: str
    doc: str


@dataclass
class Cls:
    name: str
    qualified: str
    kind: str  # "interface" or "aggregate"
    bases: list[str]
    doc: str
    members: list[Member] = field(default_factory=list)
    orderings: list[str] = field(default_factory=list)


@dataclass
class Enumerator:
    name: str
    value: int
    doc: str


@dataclass
class Enum:
    name: str
    qualified: str
    doc: str
    values: list[Enumerator] = field(default_factory=list)


TOKEN = re.compile(
    r"(?P<ns>\bnamespace\s+(?P<nsname>\w+)\s*\{)"
    r"|(?P<enum>\benum\s+class\s+(?P<ename>\w+)\s*(?::\s*[\w:]+\s*)?\{)"
    r"|(?P<cls>\b(?P<ckind>class|struct)\s+(?P<cname>\w+)\s*(?P<cbases>:[^;{}]*)?\{)"
    r"|(?P<open>\{)|(?P<close>\})"
)

METHOD = re.compile(
    r"\bvirtual\s+(?P<ret>[^;{}()]*?)\s*\b(?P<name>\w+)\s*"
    r"\((?P<params>[^;{}()]*(?:\([^()]*\)[^;{}()]*)*)\)\s*(?P<quals>[^;{}=()]*)=\s*0\s*;"
)

FIELD = re.compile(
    r"^[ \t]*(?P<type>[A-Za-z_][\w:<>, ]*?[ \t\*&]+)(?P<name>\w+)[ \t]*(?:=[^;\n]*)?;",
    re.MULTILINE,
)


class Header:
    def __init__(self, name: str):
        self.name = name
        self.text = (INCLUDE / name).read_text(encoding="utf-8")
        self.masked, self.comments = mask(self.text)

    def doc_before(self, position: int) -> str:
        """Documentation comment block ending right before `position`."""
        block: list[Comment] = []
        cursor = position
        for comment in reversed(self.comments):
            if comment.end > cursor:
                continue
            gap = self.text[comment.end:cursor]
            if gap.strip() and gap.strip() != "[[nodiscard]]":
                break
            if comment.trailing or not comment.documentation:
                break
            block.insert(0, comment)
            cursor = comment.start
        return "\n".join(c.text for c in block)

    def doc_after(self, position: int) -> str:
        """A //!< comment on the same line, after `position`."""
        line_end = self.text.find("\n", position)
        line_end = len(self.text) if line_end < 0 else line_end
        for comment in self.comments:
            if position <= comment.start < line_end and comment.trailing:
                return comment.text
        return ""

    def doc_of(self, start: int, end: int) -> str:
        return self.doc_after(end) or self.doc_before(start)

    def comments_within(self, start: int, end: int) -> list[str]:
        return [c.text for c in self.comments if start <= c.start < end]


def flatten(body: str) -> str:
    """Blank everything inside nested braces of a class body."""
    out, depth = [], 0
    for char in body:
        if char == "{":
            depth += 1
        if depth == 0 or char == "\n":
            out.append(char)
        else:
            out.append(" ")
        if char == "}":
            depth -= 1
    return "".join(out)


def split_top(text: str, separator: str = ",") -> list[tuple[int, str]]:
    """Split at top-level separators, keeping each part's offset."""
    parts, depth, start = [], 0, 0
    for i, char in enumerate(text):
        depth += char in "<(" and 1 or 0
        depth -= char in ">)" and 1 or 0
        if char == separator and depth == 0:
            parts.append((start, text[start:i]))
            start = i + 1
    parts.append((start, text[start:]))
    return parts


def base_names(declaration: str | None) -> list[str]:
    if not declaration:
        return []
    names = []
    for _, part in split_top(declaration.lstrip(":")):
        part = re.sub(r"\b(public|protected|private|virtual)\b", "", part).strip()
        if part and "<" not in part:  # template bases (ISignal<...>) are mechanism
            names.append(part.split("::")[-1])
    return names


def parse(header: Header, classes: dict, enums: dict) -> None:
    stack: list[tuple] = []
    for token in TOKEN.finditer(header.masked):
        if token.group("ns"):
            stack.append(("ns", token.group("nsname")))
        elif token.group("enum"):
            stack.append(("enum", token.group("ename"), token.end(), token.start()))
        elif token.group("cls"):
            statement = header.masked[:token.start()]
            statement = statement[max(statement.rfind(";"), statement.rfind("{"),
                                      statement.rfind("}")) + 1:]
            template = "template" in statement
            stack.append(("cls", token.group("cname"), token.end(), token.start(),
                          token.group("ckind"), token.group("cbases"), template))
        elif token.group("open"):
            stack.append(("block",))
        else:
            scope = stack.pop()
            path = [s[1] for s in stack if s[0] in ("ns", "cls")]
            if scope[0] == "enum":
                read_enum(header, scope, token.start(), path, enums)
            elif scope[0] == "cls" and not scope[6]:
                read_class(header, scope, token.start(), path, classes)


def register(table: dict, key: str, value, what: str) -> None:
    if key in table:
        raise SystemExit(f"two {what}s named {key}: {table[key].qualified} and "
                         f"{value.qualified}; give the vocabulary a distinct name")
    table[key] = value


def read_enum(header: Header, scope: tuple, end: int, path: list, enums: dict) -> None:
    _, name, start, decl = scope
    enum = Enum(name, "::".join(path + [name]), brief(header.doc_before(decl)))
    body = header.masked[start:end]
    value = -1
    for offset, part in split_top(body):
        match = re.match(r"\s*(\w+)\s*(?:=\s*(\w+))?\s*$", part)
        if not match:
            continue
        value = int(match.group(2), 0) if match.group(2) else value + 1
        position = start + offset + match.start(1)
        name_end = start + offset + match.end(1)
        doc = header.doc_after(start + offset + len(part)) or header.doc_after(name_end) \
            or header.doc_before(position)
        enum.values.append(Enumerator(match.group(1), value, brief(doc)))
    register(enums, name, enum, "enumeration")


def read_class(header: Header, scope: tuple, end: int, path: list, classes: dict) -> None:
    _, name, start, decl, kind, bases, _ = scope
    interface = name.startswith("I") and name[1:2].isupper()
    if not interface and kind != "struct":
        return
    cls = Cls(name, "::".join(path + [name]), "interface" if interface else "aggregate",
              base_names(bases), brief(header.doc_before(decl)))
    body = flatten(header.masked[start:end])
    if interface:
        for match in METHOD.finditer(body):
            params = match.group("params").strip()
            if params not in ("", "void") or "const" not in match.group("quals"):
                continue
            ret = match.group("ret").replace("[[nodiscard]]", "").strip()
            if ret == "void":
                continue
            doc = header.doc_before(start + match.start())
            cls.members.append(Member(match.group("name"), ret, brief(doc)))
    else:
        for match in FIELD.finditer(body):
            if "(" in match.group(0) or re.search(r"\b(enum|class|struct|using)\b",
                                                   match.group("type")):
                continue
            doc = header.doc_of(start + match.start(), start + match.end())
            cls.members.append(Member(match.group("name"), match.group("type"), brief(doc)))
    for text in [header.doc_before(decl)] + header.comments_within(start, end):
        found = ordering(text)
        if found and found not in cls.orderings:
            cls.orderings.append(found)
    register(classes, name, cls, "class")


# ── Building the graph ────────────────────────────────────────────────────


class Graph:
    """Nodes in insertion order; each a list of (predicate, object) pairs.

    An object is ("iri", "prefix:local" or absolute) or
    ("lit", text, language_or_datatype_or_None).
    """

    def __init__(self):
        self.nodes: dict[str, list[tuple]] = {}

    def add(self, subject: str, predicate: str, obj: tuple) -> None:
        pairs = self.nodes.setdefault(subject, [])
        if (predicate, obj) not in pairs:
            pairs.append((predicate, obj))


def iri(value: str) -> tuple:
    return ("iri", value)


def text(value: str, lang: str | None = "en") -> tuple:
    return ("lit", value, ("@" + lang) if lang else None)


def typed(value, datatype: str) -> tuple:
    return ("lit", str(value), datatype)


def resolve(cpp: str, classes: dict, enums: dict) -> tuple[str, str | None, bool]:
    """("object" | "data", range, many) for a C++ return or field type."""
    t = re.sub(r"\b(const|volatile|typename)\b", "", cpp).strip()
    many = False
    container = re.fullmatch(r"std::(vector|set|span|list)\s*<(.*)>", t)
    if container:
        many, t = True, container.group(2).strip()
    owner = re.fullmatch(r"std::(unique_ptr|shared_ptr)\s*<(.*)>", t)
    if owner:
        t = owner.group(2).strip()
    pointer = "*" in t
    t = re.sub(r"\s+", " ", t.replace("*", "").replace("&", "")).strip()
    short = t.split("::")[-1]
    if short in classes or short in enums:
        return "object", "hc:" + short, many
    if t in PRIMITIVES:
        return "data", PRIMITIVES[t], many or pointer
    return "data", None, many


def build(classes: dict, enums: dict, version: str, abi: int,
          transitions: dict[str, list[tuple[str, str]]]) -> Graph:
    g = Graph()
    onto = ONTOLOGY
    g.add(onto, "rdf:type", iri("owl:Ontology"))
    g.add(onto, "dcterms:title", text("HydroCouple interface vocabulary"))
    g.add(onto, "rdfs:comment", text(
        "The concepts of the HydroCouple component interface standard — its "
        "interfaces, enumerations, accessors and lifecycle state machines — "
        "generated from the C++ headers, which remain the normative definition."))
    g.add(onto, "owl:versionInfo", text(f"{version} (interface ABI {abi})", None))
    g.add(onto, "dcterms:license", iri("https://opensource.org/licenses/MIT"))
    g.add(onto, "rdfs:seeAlso", iri("https://github.com/HydroCouple/HydroCouple"))

    def annotation(local: str, comment: str, kind: str = "owl:AnnotationProperty"):
        g.add("hc:" + local, "rdf:type", iri(kind))
        g.add("hc:" + local, "rdfs:label", text(local, None))
        g.add("hc:" + local, "rdfs:comment", text(comment))
        g.add("hc:" + local, "rdfs:isDefinedBy", iri(onto))

    annotation("cppName", "The fully qualified C++ name the term is generated from.")
    annotation("canonicalDimensionOrdering",
               "The order of a data item's axes, as its C++ documentation states it.")
    annotation("mayTransitionTo",
               "A legal lifecycle transition (normative: Helpers::"
               "isValidComponentStatusTransition() and isValidWorkflowStatusTransition()).",
               "owl:ObjectProperty")

    for name in sorted(classes):
        cls = classes[name]
        node = "hc:" + name
        g.add(node, "rdf:type", iri("owl:Class"))
        g.add(node, "rdfs:label", text(name, None))
        if cls.doc:
            g.add(node, "rdfs:comment", text(cls.doc))
        for base in cls.bases:
            if base in classes:
                g.add(node, "rdfs:subClassOf", iri("hc:" + base))
        for sentence in cls.orderings:
            g.add(node, "hc:canonicalDimensionOrdering", text(sentence))
        g.add(node, "hc:cppName", text(cls.qualified, None))
        g.add(node, "rdfs:isDefinedBy", iri(onto))

    for name in sorted(classes):
        cls = classes[name]
        for member in sorted(cls.members, key=lambda m: m.name):
            node = f"hc:{name}.{member.name}"
            kind, target, many = resolve(member.type, classes, enums)
            g.add(node, "rdf:type", iri("owl:ObjectProperty" if kind == "object"
                                        else "owl:DatatypeProperty"))
            if not many:
                g.add(node, "rdf:type", iri("owl:FunctionalProperty"))
            g.add(node, "rdfs:label", text(member.name, None))
            if member.doc:
                g.add(node, "rdfs:comment", text(member.doc))
            g.add(node, "rdfs:domain", iri("hc:" + name))
            if target:
                g.add(node, "rdfs:range", iri(target))
            g.add(node, "hc:cppName", text(f"{cls.qualified}::{member.name}", None))
            g.add(node, "rdfs:isDefinedBy", iri(onto))

    for name in sorted(enums):
        enum = enums[name]
        node = "hc:" + name
        g.add(node, "rdf:type", iri("owl:Class"))
        g.add(node, "rdf:type", iri("skos:ConceptScheme"))
        g.add(node, "rdfs:label", text(name, None))
        if enum.doc:
            g.add(node, "rdfs:comment", text(enum.doc))
        g.add(node, "hc:cppName", text(enum.qualified, None))
        g.add(node, "rdfs:isDefinedBy", iri(onto))
        for value in enum.values:
            member = f"hc:{name}.{value.name}"
            g.add(node, "skos:hasTopConcept", iri(member))
            g.add(member, "rdf:type", iri("skos:Concept"))
            g.add(member, "rdf:type", iri(node))
            g.add(member, "skos:inScheme", iri(node))
            g.add(member, "skos:topConceptOf", iri(node))
            g.add(member, "skos:prefLabel", text(value.name, None))
            g.add(member, "skos:notation", typed(value.value, "xsd:integer"))
            if value.doc:
                g.add(member, "skos:definition", text(value.doc))
            g.add(member, "hc:cppName", text(f"{enum.qualified}::{value.name}", None))
            for source, target in transitions.get(name, []):
                if source == value.name:
                    g.add(member, "hc:mayTransitionTo", iri(f"hc:{name}.{target}"))
    return g


# ── Serializations ────────────────────────────────────────────────────────


def turtle_term(obj: tuple) -> str:
    if obj[0] == "iri":
        value = obj[1]
        return f"<{value}>" if "://" in value else value
    escaped = (obj[1].replace("\\", "\\\\").replace('"', '\\"')
               .replace("\n", "\\n").replace("\r", "\\r").replace("\t", "\\t"))
    suffix = obj[2] or ""
    if suffix and not suffix.startswith("@"):
        suffix = "^^" + suffix
    return f'"{escaped}"{suffix}'


def turtle(g: Graph) -> str:
    lines = [f"@prefix {p}: <{u}> ." for p, u in PREFIXES.items()]
    lines.append("")
    for subject, pairs in g.nodes.items():
        head = f"<{subject}>" if "://" in subject else subject
        grouped: dict[str, list[str]] = {}
        for predicate, obj in pairs:
            key = "a" if predicate == "rdf:type" else predicate
            grouped.setdefault(key, []).append(turtle_term(obj))
        body = [f"    {p} {', '.join(objs)}" for p, objs in grouped.items()]
        lines.append(head + "\n" + " ;\n".join(body) + " .")
        lines.append("")
    return "\n".join(lines)


def jsonld(g: Graph) -> str:
    context = dict(PREFIXES)
    graph = []
    for subject, pairs in g.nodes.items():
        node: dict = {"@id": subject}
        for predicate, obj in pairs:
            if predicate == "rdf:type":
                node.setdefault("@type", []).append(obj[1])
                continue
            if obj[0] == "iri":
                value = {"@id": obj[1]}
            elif obj[2] and obj[2].startswith("@"):
                value = {"@value": obj[1], "@language": obj[2][1:]}
            elif obj[2]:
                value = {"@value": obj[1], "@type": obj[2]}
            else:
                value = obj[1]
            node.setdefault(predicate, []).append(value)
        graph.append({k: (v[0] if isinstance(v, list) and len(v) == 1 else v)
                      for k, v in node.items()})
    return json.dumps({"@context": context, "@graph": graph},
                      indent=2, ensure_ascii=False) + "\n"


def page(g: Graph, classes: dict, enums: dict, version: str, abi: int) -> str:
    esc = html.escape

    def first(subject: str, predicate: str) -> str:
        for p, obj in g.nodes.get(subject, []):
            if p == predicate:
                return obj[1]
        return ""

    def objects(subject: str, predicate: str) -> list[str]:
        return [obj[1] for p, obj in g.nodes.get(subject, []) if p == predicate]

    def link(term: str) -> str:
        local = term.split(":", 1)[1] if term.startswith("hc:") else term
        if term.startswith("hc:"):
            return f'<a href="#{esc(local)}"><code>{esc(local)}</code></a>'
        return f"<code>{esc(term)}</code>"

    out = [
        "<!DOCTYPE html>", '<html lang="en">', "<head>", '<meta charset="utf-8">',
        '<meta name="viewport" content="width=device-width, initial-scale=1">',
        "<title>HydroCouple Vocabulary</title>",
        "<style>",
        ":root{--fg:#1d1d1f;--bg:#fff;--muted:#5b5b66;--rule:#e2e2e8;--code:#f3f3f6}",
        "@media (prefers-color-scheme: dark){:root{--fg:#e8e8ec;--bg:#16161a;"
        "--muted:#a0a0ab;--rule:#2e2e36;--code:#23232a}}",
        "body{font:16px/1.55 system-ui,sans-serif;color:var(--fg);background:var(--bg);"
        "max-width:60rem;margin:0 auto;padding:1.5rem 1rem}",
        "h1{margin-bottom:.2rem}h2{margin-top:2.5rem;border-bottom:1px solid var(--rule)}",
        "h3{margin:1.6rem 0 .3rem}code{background:var(--code);padding:0 .25em;border-radius:3px}",
        ".muted{color:var(--muted)}table{border-collapse:collapse;width:100%}",
        "td,th{border-bottom:1px solid var(--rule);padding:.3rem .5rem;text-align:left;"
        "vertical-align:top}a{color:inherit}",
        "</style>", "</head>", "<body>",
        "<h1>HydroCouple Vocabulary</h1>",
        f'<p class="muted">{esc(version)} · interface ABI {abi} · namespace '
        f"<code>{esc(NS)}</code></p>",
        "<p>The concepts of the HydroCouple interface standard, generated from its C++ "
        "headers (which remain normative). Machine-readable: "
        '<a href="hydrocouple.ttl">Turtle</a> · <a href="hydrocouple.jsonld">JSON-LD</a>.</p>',
        "<h2>Classes</h2>",
    ]
    for name in sorted(classes):
        node = "hc:" + name
        out.append(f'<h3 id="{esc(name)}">{esc(name)}</h3>')
        out.append(f'<p class="muted"><code>{esc(first(node, "hc:cppName"))}</code></p>')
        if first(node, "rdfs:comment"):
            out.append(f"<p>{esc(first(node, 'rdfs:comment'))}</p>")
        supers = objects(node, "rdfs:subClassOf")
        if supers:
            out.append("<p>Specializes " + ", ".join(link(s) for s in supers) + ".</p>")
        for sentence in objects(node, "hc:canonicalDimensionOrdering"):
            out.append(f"<p>Canonical dimension ordering: {esc(sentence)}</p>")
        members = sorted(m.name for m in classes[name].members)
        if members:
            out.append("<table><tr><th>Property</th><th>Range</th><th>Meaning</th></tr>")
            for member in members:
                prop = f"hc:{name}.{member}"
                rng = first(prop, "rdfs:range")
                out.append(f'<tr id="{esc(name)}.{esc(member)}"><td><code>{esc(member)}</code></td>'
                           f"<td>{link(rng) if rng else ''}</td>"
                           f"<td>{esc(first(prop, 'rdfs:comment'))}</td></tr>")
            out.append("</table>")
    out.append("<h2>Enumerations</h2>")
    for name in sorted(enums):
        node = "hc:" + name
        out.append(f'<h3 id="{esc(name)}">{esc(name)}</h3>')
        out.append(f'<p class="muted"><code>{esc(first(node, "hc:cppName"))}</code></p>')
        if first(node, "rdfs:comment"):
            out.append(f"<p>{esc(first(node, 'rdfs:comment'))}</p>")
        transitions = any(objects(f"hc:{name}.{v.name}", "hc:mayTransitionTo")
                          for v in enums[name].values)
        head = "<th>Value</th><th>Concept</th><th>Meaning</th>"
        out.append(f"<table><tr>{head}{'<th>May become</th>' if transitions else ''}</tr>")
        for value in enums[name].values:
            concept = f"hc:{name}.{value.name}"
            row = (f'<tr id="{esc(name)}.{esc(value.name)}"><td>{value.value}</td>'
                   f"<td><code>{esc(value.name)}</code></td>"
                   f"<td>{esc(first(concept, 'skos:definition'))}</td>")
            if transitions:
                row += "<td>" + ", ".join(
                    f'<a href="#{esc(t[3:])}">{esc(t.split(".")[-1])}</a>'
                    for t in objects(concept, "hc:mayTransitionTo")) + "</td>"
            out.append(row + "</tr>")
        out.append("</table>")
    out.append("<h2>Vocabulary properties</h2>")
    out.append("<table><tr><th>Property</th><th>Meaning</th></tr>")
    for local in ("mayTransitionTo", "canonicalDimensionOrdering", "cppName"):
        out.append(f'<tr id="{local}"><td><code>{local}</code></td>'
                   f"<td>{esc(first('hc:' + local, 'rdfs:comment'))}</td></tr>")
    out.append("</table>")
    out += ["</body>", "</html>", ""]
    return "\n".join(out)


# ── Entry point ───────────────────────────────────────────────────────────


def project_version() -> tuple[str, int]:
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    base = re.search(r"^\s*VERSION\s+([0-9.]+)\s*$", cmake, re.MULTILINE).group(1)
    suffix = re.search(r'set\(HYDROCOUPLE_VERSION_SUFFIX\s+"([^"]*)"\)', cmake)
    core = (INCLUDE / "hydrocouple.h").read_text(encoding="utf-8")
    abi = int(re.search(r"HYDROCOUPLE_ABI_VERSION\s*=\s*(\d+)", core).group(1))
    return base + (suffix.group(1) if suffix else ""), abi


def lifecycle_transitions() -> dict[str, list[tuple[str, str]]]:
    sys.path.insert(0, str(ROOT / "python"))
    from hydrocouple import core, helpers  # noqa: E402  (the parity-tested mirror)

    tables = {
        "ComponentStatus": (core.ComponentStatus,
                            helpers.is_valid_component_status_transition),
        "WorkflowStatus": (core.WorkflowStatus,
                           helpers.is_valid_workflow_status_transition),
    }
    return {name: [(a.name, b.name) for a in enum for b in enum if legal(a, b)]
            for name, (enum, legal) in tables.items()}


def generate() -> dict[str, str]:
    """The three output files, by name."""
    classes: dict[str, Cls] = {}
    enums: dict[str, Enum] = {}
    for name in HEADERS:
        parse(Header(name), classes, enums)
    version, abi = project_version()
    graph = build(classes, enums, version, abi, lifecycle_transitions())
    return {
        "hydrocouple.ttl": turtle(graph),
        "hydrocouple.jsonld": jsonld(graph),
        "index.html": page(graph, classes, enums, version, abi),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--check", action="store_true",
                        help="exit 1 if the files under docs/vocabulary are stale")
    args = parser.parse_args()
    outputs = generate()
    stale = [name for name, content in outputs.items()
             if not (OUTPUT / name).exists()
             or (OUTPUT / name).read_text(encoding="utf-8") != content]
    if args.check:
        for name in stale:
            print(f"stale: docs/vocabulary/{name} (run python tools/generate_vocabulary.py)")
        return 1 if stale else 0
    OUTPUT.mkdir(parents=True, exist_ok=True)
    for name, content in outputs.items():
        (OUTPUT / name).write_text(content, encoding="utf-8")
    print(f"wrote {len(outputs)} files to docs/vocabulary/")
    return 0


if __name__ == "__main__":
    sys.exit(main())
