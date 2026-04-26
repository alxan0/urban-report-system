# Utilizare AI

**Tool folosit**: Claude Sonnet 4.6

## Prompt
I am working on a C CLI project called city_manager. Reports are stored in a binary file (reports.dat), and each record uses a Report struct defined in report.h with fields like category (string), inspector (string), severity (int), and timestamp (time_t). The filter command is called as --filter <district> <condition...> and each condition has format field:operator:value. Supported fields are severity, category, inspector, timestamp. Supported operators are ==, !=, <, <=, >, >=. I already implemented the main command flow and file iteration logic (open/read/loop/print), and I only want help for two helper functions: int parse_condition(const char *input, char *field, char *op, char *value); and int match_condition(Report *r, const char *field, const char *op, const char *value); Please generate clean C code for these two functions with strict validation and clear behavior for invalid input. parse_condition must split safely, validate field/operator, and reject malformed strings. match_condition must perform type-correct comparisons for numeric vs string fields. I also attached other project files so you have full context about architecture, command flow, and data structures

## Ce a generat AI-ul
Pe langa cele doua functii cerute (`parse_condition` si `match_condition`), AI-ul a generat in plus trei functii ajutatoare pe care le-am pastrat in `filter.c`


## Ce am modificat eu
- am adaugat o functie separata care verifica ca `strtol` a reusit si ca a consumat tot sirul

## Ce a gresit AI-ul
- Folosea `atoi` in loc de o conversie stricta, ceea ce nu detecteaza input invalid (am rezolvat cu `parse_long_strict`)

## Ce am invatat
Am invatat ca AI-ul imi poate oferi un cod destul de bun din start, dar cu cat implementarea se bazeaza pe detalii mai fine, chiar si cu context destul de amplu, inevitabil vor fi lucruri pe care le va rata.

## Concluzie
In concluzie, AI-ul m-a ajutat la scheletul initial, dar fara contextul amplu si fisierele atasate, acesta nu ar fi fost in stare sa produca un punct de plecare atat de bun. Totusi, acesta a avut niste mici scapari, a trebuit sa implementez manual o functie pentru verificarea lui `strtol`, care de asemenea are rolul de a detecta anumite inputuri gresite cum ar fi "123adb".