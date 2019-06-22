# Reproduce the code in README.md.
import ctok
tok = ctok.CTok(b"(hello+world)")
for token in tok:
    print(token)
