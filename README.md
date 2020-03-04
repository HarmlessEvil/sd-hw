# sd-hw
Homeworks for Software Design course

# Structure
## Pipeline

1. Get input;
2. Parse input;
3. Execute;
4. Repeat

## 1. Get input

```
std::string input;
std::cin >> input;
```

## 2. Parse input

### ```std::string -> exec_list ```

```
exec_list:
  vector<exec_unit>
  
exec_unit:
  vector<token> (aka tokens)
  
class tkn_t:
  symb_t
  std::string (aka token)
  
enum symb_t:
  eq -> =
  var -> $
  dbrace -> "..."
  sbrace -> '...'
  pipe -> |
  white -> 
  els -> _
```
### Семантика

* Токен -- слово, интерпретируемое как команда.
* ```exec_unit``` -- последовательность токенов, обрабатываемых вместе (то, что между pipe'ами);
* ```exec_list``` -- набор единиц исполнения, исполняемых вместе.

### Ключевые функции

```std::optional<cli::exec_list> lexparse(std::string str, cli::env_t &env);```

* Всеобъемлющая функция, разбивающая всю строку на ```exec_unit```ы.
* Возвращает ```exec_list``` или ничего, если в какой-то момент были нарушены синтаксические правила.

```std::optional<tokens> tokenize(const std::string in);```

* implementation-only функция для разбиения на токены;

```std::optional<tokens> preprocess(std::optional<tokens> toks, env_t &env);```
* замена переменных и удаление пустых символов;

```std::optional<exec_list> split_exec(std::optional<tokens> toks)```
* разбиение по ```pipe```ам.

## 3. Execute

Команды -- свободные функции, принимающие ```exec_unit```ы.
Подбор подходящей ```exec_unit```у функции происходит посредствов сравнения с образцом (switch case).

```
enum builtin_t
    eq 
    pwd
    cat
    wc
    echo
    ext
    els

```
Командам, вызов которых будет делегироваться ОС, соответсвует builtin_t::els.

## pipes 

```fork``` и```execvp```. 

  
  
