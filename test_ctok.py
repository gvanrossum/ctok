from token import *
import ctok

def test_basic():
    input = b"(hello+world)"
    tokens = list(ctok.CTok(input))
    assert tokens == [
        (LPAR, b'(', (1, 0), (1, 1)),
        (NAME, b'hello', (1, 1), (1, 6)),
        (PLUS, b'+', (1, 6), (1, 7)),
        (NAME, b'world', (1, 7), (1, 12)),
        (RPAR, b')', (1, 12), (1, 13)),
    ]

def test_indent():
    input = b"if 1:\n  pass\npass"
    tokens = list(ctok.CTok(input))
    assert tokens == [
        (NAME, b'if', (1, 0), (1, 2)),
        (NUMBER, b'1', (1, 3), (1, 4)),
        (COLON, b':', (1, 4), (1, 5)),
        (NEWLINE, b'', (1, 5), (1, 5)),
        (INDENT, None, (2, -1), (2, -1)),
        (NAME, b'pass', (2, 2), (2, 6)),
        (NEWLINE, b'', (2, 6), (2, 6)),
        (DEDENT, None, (3, -1), (3, -1)),
        (NAME, b'pass', (3, 0), (3, 4)),
    ]
