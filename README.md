CPython's tokenizer exposed as a Python class
=============================================

```
>>> import ctok
>>> tok = ctok.CTok(b"(hello+world)")
>>> for token in tok: print(token)
...
(7, b'(')
(1, b'hello')
(14, b'+')
(1, b'world')
(8, b')')
>>>
```

TODO
----

- Return line number and column offset
- Support reading from a file/stream
- Support str instead of (or in addition to) bytes?
