# The git lock — RESOLVED 2026-09-20

**Status: done. Nothing here is outstanding.** Kept as the record of what
happened, because the failure mode was not the one this file originally
described and the next person to meet it should not have to rediscover that.

---

## What it was

The repository lives on a mount that denies `unlink` inside `.git/`. Git
creates a lock file for every write and removes it afterwards; here the
removal silently failed, so every lock git had ever taken was still lying
there, and each one blocked the *next* operation that needed it.

By the time this was cleared there were three:

| File | Size | Left by |
|---|---|---|
| `.git/index.lock` | 0 | an interrupted `git add`, 2026-09-19 20:26 |
| `.git/HEAD.lock` | 0 | the U1/U6/U3 commit `584679c`, which **succeeded** |
| `.git/refs/heads/v2.lock` | 41 | a `git update-ref` that failed on `HEAD.lock` |

The second is the one worth remembering: **a successful commit left a lock
that blocked the next commit.** The work was not lost and nothing was
corrupt; git simply could not take a lock it already held from a process
that had long since exited.

## How the U-phase commits were made before it was cleared

`c98b456` and `584679c` were committed through a private index —
`GIT_INDEX_FILE=$HOME/u_phase_index`, `git read-tree HEAD`, explicit-path
`git add`, then `git commit`. That avoided `.git/index.lock` and worked
twice. It stopped working when `HEAD.lock` appeared, because updating the
branch that `HEAD` points at locks `HEAD` too, and no choice of index file
gets around that.

## How it was cleared

Deletion inside a connected folder is off by default and `rm` fails with
`Operation not permitted` until the user allows it. Permission was
requested and granted for the repository folder, then:

```bash
# Each confirmed to be a lock file, not content, and no rebase or merge
# in flight — .git/rebase-merge, .git/rebase-apply, .git/MERGE_HEAD and
# .git/CHERRY_PICK_HEAD all absent.
rm -f .git/index.lock .git/HEAD.lock .git/refs/heads/v2.lock

# The index on disk was from before the last two commits, so it had to be
# refreshed against HEAD. Mixed, which touches the index only.
git reset -q
```

`.git/index` was confirmed present afterwards, `git status` read correctly,
and `c67e6c6` (U4) committed normally.

> **Never `git reset --hard` in this tree.** It was true then and it is true
> now: there is uncommitted and untracked work here — `plans/`,
> `verification/`, `tests/manual/` and
> `tests/fixtures/mesh/domain.composer.json` are all untracked by
> convention, and `--hard` plus a stray `-d` would take them.

## If it comes back

It will, if a git process is ever interrupted on this mount again. The
recipe is the three steps above: confirm each `.lock` is a lock and not
content, confirm no rebase or merge is in flight, remove them, then
`git reset -q` and check `.git/index` still exists. Deletion permission
lasts only for the session that asked for it, so a new session has to ask
again.
