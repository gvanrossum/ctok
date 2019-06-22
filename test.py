import ctok

input = b"(the + quick / '''brown\nfox''' and async await jumped?)\n"
tok = ctok.CTok(input)
for token in iter(tok.get, (0, None)):
    print(token)
