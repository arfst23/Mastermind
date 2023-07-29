# Mastermmind

[Mastermind](https://en.wikipedia.org/wiki/Mastermind_(board_game)) is a code-breaking game.
The game is played by two players the codemaker and the codebreaker.
The codemaker chooses a pattern of four code pegs (duplicates are
allowed).  The chosen pattern is placed in the four holes covered by
the shield, visible to the codemaker but not to the codebreaker.
 
The codebreaker tries to guess the pattern, in both order and color,
within ten turns. Each guess is made by placing a row of code pegs on the
decoding board. Once placed, the codemaker provides feedback by placing
from zero to four key pegs in the small holes of the row with the guess. A
colored or black key peg is placed for each code peg from the guess
which is correct in both color and position. A white key peg indicates
the existence of a correct color code peg placed in the wrong position.

The game can be played with 6, 7, or 8 different colors.
An easier variant adds the constraint, that colors are only allowed to
occur once in the code

## Optimality

The *minimum* criterion focuses on the maximum number of guesses:
1. minimal maximum number of guesses
2. minimal average number of guesses

The *average* criterion focuses on the average number of guesses:
1. minimal average number of guesses
2. minimal maximum number of guesses

A codemaker would select the type of guess that would on average take
the longest to solve. To compensate for that, the criterion minimal
average number of guesses is changed to minimum maximal (among the ﬁve
code classes) average number of guesses.

It turns out that the strategies optimal wrt. to this minmax criterion
are a subset of the strategies that are optimal wrt. initial formulation.
So we add:

3. minimum maximal (among the ﬁve code classes) average number of guesses.

## Solutions

Computing time on a Intel Core i7-9700K.

| Criterion | Colors | maximal | average | minmax average | Time   |
| --------- | ------ | ------- | ------- | -------------- | ------ |
| minimum   |    6   |    5    | 4.34028 | 4.34306        |    0.5 |
| average   |    6   |    6    | 4.34105 | 4.34167        |    0.5 |
| min/avg   |    7   |    6    | 4.67638 | 4.67738        |  150.0 |
| minimum   |    8   |    6    | 4.99829 | 5.07143        | 8000.0 |
| average   |    8   |    7    | 4.99780 | 5.07540        | 8000.0 |

## File Format

The strategies are stored in two formats

### Punch Card

| Column | Description                    |
| ------ | ------------------------------ |
| 1-3    | Line number                    |
| 5      | Number of guess                |
| 7-10   | Guess                          |
| 12-... | For every codemaker's feddback |
|        | `>123` next line number        |
|        | `1234` code                    |

### YML

An array with 2 elements
1. the guess
2. a hash with
    - key, the codemaker's feedback
    - value, either
        - the code
        - the next guess in the same - recursive - format