CPython's tokenizer exposed as a Python class
=============================================

```
>>> import ctok
>>> tok = ctok.CTok(b"(hello+world)")
>>> tok.get()
(7, b'(')
>>> tok.get()
(1, b'hello')
>>> tok.get()
(14, b'+')
>>> tok.get()
(1, b'world')
>>> tok.get()
(8, b')')
>>> tok.get()
(0, None)
>>> 
```

TODO
----

- Return line number and column offset
- Make it usable as an iterator
- Support cloning CTok objects (to support packrat parsing)
