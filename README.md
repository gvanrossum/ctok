CPython's tokenizer exposed as a Python class
=============================================

```
>>> import ctok
>>> tok = ctok.CTok(b"(hello+world)")
>>> for token in tok: print(token)
...
(7, b'(', (1, 0), (1, 1))
(1, b'hello', (1, 1), (1, 6))
(14, b'+', (1, 6), (1, 7))
(1, b'world', (1, 7), (1, 12))
(8, b')', (1, 12), (1, 13))
>>>
```

TODO
----

- Support reading from a file/stream
- Support str instead of (or in addition to) bytes?
