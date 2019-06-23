import sys
from token import *

import pytest

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

def test_no_indent():
    input = b"(foo\n  bar)"
    tokens = list(ctok.CTok(input))
    assert tokens == [
        (LPAR, b'(', (1, 0), (1, 1)),
        (NAME, b'foo', (1, 1), (1, 4)),
        # No NEWLINE, INDENT here!
        (NAME, b'bar', (2, 2), (2, 5)),
        (RPAR, b')', (2, 5), (2, 6)),
    ]

def test_multi_line_string():
    input = b"'''foo\nbar'''"
    tokens = list(ctok.CTok(input))
    if sys.version_info >= (3, 8):
        start = (1, 0)
    else:
        # Older Python versions don't have the correct line number for
        # the start of a multi-line string.
        start = (2, -1)
    assert tokens == [
        (STRING, b"'''foo\nbar'''", start, (2, 6)),
    ]

def test_input_cr():
    input = b"foo\rbar"
    tok = ctok.CTok(input)
    assert tok.input == b"foo\nbar"

def test_input_crlf():
    input = b"foo\r\nbar"
    tok = ctok.CTok(input)
    assert tok.input == b"foo\nbar"

def test_encoding():
    input = b"# coding: latin-1\nfoo\nbar"
    tok = ctok.CTok(input)
    assert tok.encoding == "iso-8859-1"

def test_encoding_default():
    input = b"foo\nbar"
    tok = ctok.CTok(input)
    assert tok.encoding is None

def test_get_raw():
    input = b"foo bar\r\nbaz"
    tok = ctok.CTok(input)
    assert tok.get_raw() == (NAME, 0, 3)
    assert tok.get_raw() == (NAME, 4, 7)
    assert tok.get_raw() == (NEWLINE, 7, 7)
    assert tok.get_raw() == (NAME, 8, 11)
    assert tok.get_raw() == (ENDMARKER, -1, -1)

def test_endmarker():
    input = b"foo\nbar\n"
    tok = ctok.CTok(input)
    tok.get()
    tok.get()
    assert tok.get() == (NAME, b"bar", (2, 0), (2, 3))
    assert tok.get() == (NEWLINE, b"", (2, 3), (2, 3))
    with pytest.raises(StopIteration) as excinfo:
        tok.get()
    assert "end of input at line 2" in str(excinfo.value)

def test_error():
    input = b"foo\n'bar"
    tok = ctok.CTok(input)
    assert tok.get() == (NAME, b"foo", (1, 0), (1, 3))
    assert tok.get()[0] == NEWLINE
    with pytest.raises(SyntaxError) as excinfo:
        tok.get()
    assert "error at line 2" in str(excinfo.value)
