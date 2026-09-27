# Security policy

## Supported versions

LCDHost has no stable release yet. Security fixes go to the `master` branch
and are available in the next CI build.

## Reporting a vulnerability

Please **do not open a public issue** for security problems.

Report them privately through GitHub:
[Security → Report a vulnerability](https://github.com/LokLakh-s/LCDHost/security/advisories/new).
Include a description of the problem, the affected version or commit, and
steps to reproduce if you have them.

You should get a first answer within two weeks. Once a fix is available, the
advisory is published and the reporter is credited unless they prefer not to
be.

## Scope and known risks

LCDHost loads native plugins and layouts. Keep in mind that:

- **Plugins are native code** running with your user's privileges. Only load
  plugins you trust.
- **Layouts can start programs and open URLs**: the Cursor plugin supports
  actions that launch an executable named in the layout. Only use layouts
  from sources you trust.
- The DataViewer plugin reads the memory of other processes (games) to
  display their data. Some anti-cheat systems may flag this.

The online update mechanism inherited from the original project downloaded
and ran unsigned code over plain HTTP. It was removed in version 0.0.41.
