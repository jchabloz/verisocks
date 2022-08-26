# How to view CUnit-generated XML output files

## Solution 1: Use local HTTP server

Link the required `.xsl` document(s) from `/usr/share/CUnit` in the same folder as the desired `result.xml` file.

From the folder in question, launch a simple HTTP server using the Python http module:
   
```sh
python3 -m http.server
```

This is required to avoid cross origin request issue (CORS) that makes any
modern browser reject the xslt stylesheet if coming from the local filesystem.
Both xml and xslt documents have to be served from the same origin. Lauching
the simple Python test server meets this purpose.

## Solution 2: Use xsltproc XSLT CLI processor

This is even simpler, actually...

Just run:

```sh
xsltproc <your_local_path>/CUnit-Run.xsl result.xml > result.html
firefox result.html
```
