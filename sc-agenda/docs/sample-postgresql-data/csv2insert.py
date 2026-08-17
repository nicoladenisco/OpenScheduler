#!/usr/bin/env python3
"""Genera istruzioni INSERT SQL a partire da un file CSV.

Esempio:
    python3 csv2insert.py risorse.csv --table risorse
    python3 csv2insert.py risorse_link.csv --table risorse_link --batch 100 -o link.sql
"""

import argparse
import csv
import re
import sys

IDENTIFIER_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_$]*$")
NUMBER_RE = re.compile(r"^[+-]?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$")


def check_identifier(name, kind):
    if not IDENTIFIER_RE.match(name):
        raise SystemExit(f"{kind} non valido: {name!r}")
    return name


def is_numeric_column(name):
    lowered = name.lower()
    return lowered.startswith("id_") or lowered.endswith("_id")


def sql_literal(value, column, null_tokens):
    if value is None:
        return "NULL"
    stripped = value.strip()
    if stripped in null_tokens:
        return "NULL"
    if is_numeric_column(column):
        if not NUMBER_RE.match(stripped):
            raise SystemExit(f"Valore non numerico nella colonna {column}: {value!r}")
        return stripped
    return "'" + value.replace("'", "''") + "'"


def build_statements(rows, table, columns, batch_size, on_conflict, null_tokens):
    col_list = ", ".join(columns)
    prefix = f"INSERT INTO {table} ({col_list}) VALUES"
    suffix = (" " + on_conflict if on_conflict else "") + ";"

    buffer = []
    for row in rows:
        values = ", ".join(sql_literal(v, c, null_tokens) for c, v in zip(columns, row))
        buffer.append(f"    ({values})")
        if len(buffer) >= batch_size:
            yield prefix + "\n" + ",\n".join(buffer) + suffix
            buffer = []
    if buffer:
        yield prefix + "\n" + ",\n".join(buffer) + suffix


def main(argv=None):
    parser = argparse.ArgumentParser(description="Converte un CSV in istruzioni INSERT SQL.")
    parser.add_argument("csvfile", help="file CSV di input")
    parser.add_argument("--table", required=True, help="nome della tabella di destinazione")
    parser.add_argument("--schema", help="schema della tabella (opzionale)")
    parser.add_argument("--columns", help="elenco colonne separate da virgola (default: header del CSV)")
    parser.add_argument("--delimiter", default=",", help="separatore di campo del CSV (default: ,)")
    parser.add_argument("--encoding", default="utf-8", help="encoding del CSV (default: utf-8)")
    parser.add_argument("--batch", type=int, default=1, help="righe per singolo INSERT (default: 1)")
    parser.add_argument("--null", dest="null_tokens", default="NULL,\\N,",
                        help="valori interpretati come NULL, separati da virgola (default: 'NULL,\\N,')")
    parser.add_argument("--on-conflict", help="clausola finale, es. \"ON CONFLICT DO NOTHING\"")
    parser.add_argument("--truncate", action="store_true",
                        help="emette un TRUNCATE della tabella prima degli INSERT")
    parser.add_argument("-o", "--output", help="file SQL di output (default: stdout)")
    args = parser.parse_args(argv)

    if args.batch < 1:
        raise SystemExit("--batch deve essere >= 1")

    qualified = check_identifier(args.table, "Nome tabella")
    if args.schema:
        qualified = check_identifier(args.schema, "Schema") + "." + qualified

    null_tokens = set(args.null_tokens.split(","))

    with open(args.csvfile, newline="", encoding=args.encoding) as fh:
        reader = csv.reader(fh, delimiter=args.delimiter)
        try:
            header = next(reader)
        except StopIteration:
            raise SystemExit("CSV vuoto")

        columns = [c.strip() for c in (args.columns.split(",") if args.columns else header)]
        for col in columns:
            check_identifier(col, "Nome colonna")

        rows = (row for row in reader if any(field.strip() for field in row))
        statements = build_statements(rows, qualified, columns, args.batch,
                                      args.on_conflict, null_tokens)

        out = open(args.output, "w", encoding="utf-8") if args.output else sys.stdout
        try:
            if args.truncate:
                out.write(f"TRUNCATE TABLE {qualified};\n")
            count = 0
            for stmt in statements:
                out.write(stmt + "\n")
                count += 1
            if not count:
                print("Attenzione: nessuna riga di dati trovata.", file=sys.stderr)
        finally:
            if args.output:
                out.close()


if __name__ == "__main__":
    main()
