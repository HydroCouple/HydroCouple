# -*- coding: utf-8 -*-
#
# HydroCouple Python bindings documentation build configuration file.
#
# Modeled on the openswmm.engine Sphinx setup: pydata-sphinx-theme, the
# same extension set, and the same co-deployment cross-linking layout —
#     /          Doxygen C++ interface docs (site root)
#     /python/   these (Sphinx) docs

import os
import sys

# Add the python package directory to sys.path so autodoc can find the
# pure-Python `hydrocouple` package.
_PACKAGE_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, _PACKAGE_ROOT)

try:
    import hydrocouple
    version = hydrocouple.__version__
    release = hydrocouple.__version__
except Exception:
    version = '2.0.0'
    release = version

# -- General configuration ------------------------------------------------

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.napoleon',
    'sphinx.ext.intersphinx',
    'sphinx.ext.viewcode',
    'sphinx.ext.todo',
    'sphinx.ext.mathjax',
    'myst_parser',
    # sphinx-design: grid / grid-item-card directives used on the landing
    # page, matching the openswmm.engine docs.
    'sphinx_design',
    # sphinxcontrib-mermaid: diagrams rendered client-side, matching the
    # Doxygen side of the site.
    'sphinxcontrib.mermaid',
]

# MyST (Markdown) settings
myst_enable_extensions = [
    'colon_fence',
    'deflist',
]
myst_heading_anchors = 3
suppress_warnings = ['myst.header', 'ref.python', 'duplicate']
source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}

master_doc = 'index'

# -- Project information --------------------------------------------------

project = 'HydroCouple'
copyright = '2026 Caleb Buahin'
author = 'Caleb Buahin'

# -- Napoleon settings (Google/NumPy docstrings) --------------------------

napoleon_google_docstring = True
napoleon_numpy_docstring = True
napoleon_include_init_with_doc = True
napoleon_use_param = True
napoleon_use_rtype = True

# -- Autodoc settings -----------------------------------------------------
# The `hydrocouple` package is pure Python, so autodoc imports it
# directly.  Only the compiled Cython bridge (`_hydrocouple.*`) is mocked
# so the docs build without a C++20 toolchain.

autodoc_default_options = {
    'members': True,
    'undoc-members': True,
    'show-inheritance': True,
    'member-order': 'bysource',
}
autodoc_typehints = 'description'
autodoc_typehints_format = 'short'

autodoc_mock_imports = [
    '_hydrocouple',
    '_hydrocouple._core',
    '_hydrocouple._temporal',
    '_hydrocouple._spatial',
    '_hydrocouple._spatiotemporal',
]

autoclass_content = 'both'

autosummary_generate = True

add_function_parentheses = True
add_module_names = False

numfig = True

# -- Intersphinx ----------------------------------------------------------

intersphinx_mapping = {
    'python': ('https://docs.python.org/3', None),
    'numpy': ('https://numpy.org/doc/stable/', None),
}

# -- Options for HTML output ----------------------------------------------

language = 'en'
pygments_style = 'sphinx'
todo_include_todos = True

exclude_patterns = [
    '_build',
]

on_rtd = os.environ.get('READTHEDOCS', None) == 'True'
if not on_rtd:
    html_theme = 'pydata_sphinx_theme'
else:
    html_theme = 'default'

html_theme_options = {
    "logo": {
        "text": "HydroCouple",
        "image_light": "images/hydrocouple_logo.png",
        "image_dark": "images/hydrocouple_logo.png",
    },
    # Top-nav cross-link back to the Doxygen C++ interface docs.
    # Deployment layout (see .github/workflows/build_docs.yml):
    #     /         Doxygen docs (root)
    #     /python/  these (Sphinx) docs
    # ``../index.html`` resolves to the Doxygen root when both sites are
    # co-deployed under GitHub Pages.  When viewing the Sphinx docs
    # locally without the Doxygen bundle, this link 404s — that's an
    # accepted local-only limitation.
    "external_links": [
        {"name": "C++ Interface Docs", "url": "../index.html"},
    ],
    "icon_links": [
        {
            "name": "GitHub",
            "url": "https://github.com/HydroCouple/HydroCouple",
            "icon": "fa-brands fa-github",
            "type": "fontawesome",
        },
        {
            "name": "PyPI",
            "url": "https://pypi.org/project/hydrocouple",
            "icon": "fa-brands fa-python",
            "type": "fontawesome",
        },
    ],
    "use_edit_page_button": False,
    "show_toc_level": 2,
    "navbar_end": ["theme-switcher.html", "navbar-icon-links.html"],
}

html_logo = 'images/hydrocouple_logo.png'
html_title = 'HydroCouple'
html_favicon = 'images/hydrocouple_logo.png'

html_static_path = ['_static']
html_css_files = ['hydrocouple.css']

htmlhelp_basename = 'hydrocoupledoc'

# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
    'papersize': 'letterpaper',
    'pointsize': '10pt',
}

latex_documents = [
    (master_doc, 'hydrocouple.tex', 'HydroCouple Documentation', author,
     'manual'),
]

# -- Options for manual page output ---------------------------------------

man_pages = [
    (master_doc, 'hydrocouple', 'HydroCouple Documentation', [author], 1)
]
