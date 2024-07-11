# RAL

**Render Abstraction Layer** is a thin abstraction over graphics APIs. Everything in `render` builds on top of the code in
this folder in order to be API-agnostic. It also makes writing graphics code easier as it smooths over some of the discrepancies
between APIs like texture/buffer creation and updating shader values.