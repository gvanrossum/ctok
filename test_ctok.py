import ctok

def test_basic():
    input = b"(hello+world)"
    tokens = list(ctok.CTok(input))
    assert tokens == [
        (7, b'(', (1, 0), (1, 1)),
        (1, b'hello', (1, 1), (1, 6)),
        (14, b'+', (1, 6), (1, 7)),
        (1, b'world', (1, 7), (1, 12)),
        (8, b')', (1, 12), (1, 13)),
    ]
