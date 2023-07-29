//******************************************************************************
// >>> config

// 4 6 5
// 4 6 6
// 4 7 6
// 4 8 6

#define POSITIONS 4
#define COLORS 6
#define DEPTH_MAX 5

// <<<
//******************************************************************************
// >>> header

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <assert.h>

#define ift(expr) if (__builtin_expect(expr, 1))
#define iff(expr) if (__builtin_expect(expr, 0))

#if COLORS >= 8
#define STATIC_DATA
#endif

typedef uint8_t Bool;
typedef uint8_t Depth;
typedef uint8_t Color;

#if COLORS >= 8
typedef uint32_t Size;
#else
typedef uint16_t Size;
#endif

typedef uint32_t Score;
#define SCORE_INF UINT32_MAX

#define PERMUTATIONS (COLORS * COLORS * COLORS * COLORS)
typedef uint16_t Permutation;

#define COLUMNS 14
typedef uint8_t Column;

#define POSFAC (2 * 3 * 4)

#if COLORS == 6
#define COLORFAC (2 * 3 * 4 * 5 * 6)
#endif
#if COLORS == 7
#define COLORFAC (2 * 3 * 4 * 5 * 6 * 7)
#endif
#if COLORS == 8
#define COLORFAC (2 * 3 * 4 * 5 * 6 * 7 * 8)
#endif

// 4x6 =>   17280
// 4x7 =>  120960
// 4x8 =>  967680
// 5x8 => 4838400
#define SYMMETRIES (POSFAC * COLORFAC)
typedef uint32_t Symmetry;

#define SOLUTIONS 8000

typedef uint16_t Row;
// row is coded as row, solution is coded as solution | ROWSOL
#define ROWSOL 0x8000

typedef struct
{
  Depth depth; // depth of guess 0 ..
  Permutation permutation; // current permutation
  Score score; // score of the subtree
  Depth max; // max depth in subtree
  Score convex; // convex score of the subtree
  Row rows; // number of rows in solution

  Row row; // calculation: next in bucket, print: row in breath first order
  Row solution[COLUMNS]; // solution for given eval
} Solution;

// <<<
//******************************************************************************
// >>> Column eval(Permutation, Permutation)

// 4x6 =>  825k
// 4x7 =>   3M
// 4x8 =>   8M
// 5x8 => 512M
static Column *relation[PERMUTATIONS];

static void mkeval()
{
  { // build lower triangular matrix
    static Column rel[PERMUTATIONS * (PERMUTATIONS + 1) / 2];
    *relation = rel;
    for (Permutation permutation = 1; permutation < PERMUTATIONS; permutation++)
      relation[permutation] = relation[permutation - 1] + permutation;
    assert(relation[PERMUTATIONS - 1] - rel
      == PERMUTATIONS * (PERMUTATIONS + 1) / 2 - PERMUTATIONS);
  }

  Permutation permutation1 = 0;
  for (Color color11 = 0; color11 < COLORS; color11++)
    for (Color color12 = 0; color12 < COLORS; color12++)
      for (Color color13 = 0; color13 < COLORS; color13++)
	for (Color color14 = 0; color14 < COLORS; color14++)
	{
	  Permutation permutation2 = 0;
	  for (Color color21 = 0; color21 < COLORS; color21++)
	    for (Color color22 = 0; color22 < COLORS; color22++)
	      for (Color color23 = 0; color23 < COLORS; color23++)
		for (Color color24 = 0; color24 < COLORS; color24++)
		{
		  iff (permutation2 > permutation1) // lower triangular
		    goto xbreak;

		  // determine white and black
#if COLORS == 6
		  int color1[] = { 0, 0, 0, 0, 0, 0 };
		  int color2[] = { 0, 0, 0, 0, 0, 0 };
#endif
#if COLORS == 7
		  int color1[] = { 0, 0, 0, 0, 0, 0, 0 };
		  int color2[] = { 0, 0, 0, 0, 0, 0, 0 };
#endif
#if COLORS == 8
		  int color1[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		  int color2[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
#endif
		  int white = 0;

		  iff (color11 == color21)
		    white++;
		  else
		    color1[color11]++, color2[color21]++;

		  iff (color12 == color22)
		    white++;
		  else
		    color1[color12]++, color2[color22]++;

		  iff (color13 == color23)
		    white++;
		  else
		    color1[color13]++, color2[color23]++;

		  iff (color14 == color24)
		    white++;
		  else
		    color1[color14]++, color2[color24]++;

		  int black = (color1[0] < color2[0] ? color1[0] : color2[0])
		    + (color1[1] < color2[1] ? color1[1] : color2[1])
		    + (color1[2] < color2[2] ? color1[2] : color2[2])
		    + (color1[3] < color2[3] ? color1[3] : color2[3])
		    + (color1[4] < color2[4] ? color1[4] : color2[4])
#if COLORS == 6
		    + (color1[5] < color2[5] ? color1[5] : color2[5]);
#endif
#if COLORS == 7
		    + (color1[5] < color2[5] ? color1[5] : color2[5])
		    + (color1[6] < color2[6] ? color1[6] : color2[6]);
#endif
#if COLORS == 8
		    + (color1[5] < color2[5] ? color1[5] : color2[5])
		    + (color1[6] < color2[6] ? color1[6] : color2[6])
		    + (color1[7] < color2[7] ? color1[7] : color2[7]);
#endif

#define X COLUMNS
		  static Column result[5][5] =
		  {         /* w=0 w=1 w=2 w=3 w=4 */
		    /* b=0 */ {  0,  1,  2,  3,  4 },
		    /* b=1 */ {  5,  6,  7,  8,  X },
		    /* b=2 */ {  9, 10, 11,  X,  X },
		    /* b=3 */ { 12,  X,  X,  X,  X },
		    /* b=4 */ { 13,  X,  X,  X,  X }
		  };
#undef X

		  Column column = result[white][black];
		  assert(column != COLUMNS);
		  relation[permutation1][permutation2] = column;
		  permutation2++;
		}
	  xbreak:
	    permutation1++;
	}
}

static inline Column eval(Permutation permutation1, Permutation permutation2)
{
  return (Column)
  (
    permutation1 > permutation2
      ? relation[permutation1][permutation2]
      : relation[permutation2][permutation1]
  );
}

// <<<
//******************************************************************************
// >>> Permutation permutate(Symmetry, Permutation)

// 4x6 =>    2M
// 4x7 =>   24M
// 4x8 =>  315M
// 5x8 => 1267M
static Permutation psymmetry[POSFAC][PERMUTATIONS]; // symmetry by position
static Permutation csymmetry[COLORFAC][PERMUTATIONS]; // symmetry by color

static void mksymmetry()
{
  Color i[COLORS]; // COLORS >= POSITIONS
  Symmetry symmetry;

  for (i[0] = 0, symmetry = 0; i[0] < POSITIONS; i[0]++) // index of symmetry op
    for (i[1] = 0; i[1] < POSITIONS; i[1]++)
    {
      iff (i[1] == i[0])
	continue;
      for (i[2] = 0; i[2] < POSITIONS; i[2]++)
      {
	iff (i[2] == i[0] || i[2] == i[1])
	  continue;
	for (i[3] = 0; i[3] < POSITIONS; i[3]++)
	{
	  iff (i[3] == i[0] || i[3] == i[1] || i[3] == i[2])
	    continue;

	  Color color[POSITIONS];
	  Permutation perm;
	  for (color[0] = 0, perm = 0; color[0] < COLORS; color[0]++) // digit of perm
	    for (color[1] = 0; color[1] < COLORS; color[1]++)
	      for (color[2] = 0; color[2] < COLORS; color[2]++)
		for (color[3] = 0; color[3] < COLORS; color[3]++)
		{
		  // encode result
		  psymmetry[symmetry][perm] = (Permutation)
		  (
		    COLORS * COLORS * COLORS * color[i[0]]
		    + COLORS * COLORS * color[i[1]]
		    + COLORS * color[i[2]]
		    + color[i[3]]
		  );
		  perm++;
		}
	  symmetry++;
	}
      }
    }

  for (i[0] = 0, symmetry = 0; i[0] < COLORS; i[0]++) // index of symmetry op
    for (i[1] = 0; i[1] < COLORS; i[1]++)
    {
      iff (i[1] == i[0])
	continue;
      for (i[2] = 0; i[2] < COLORS; i[2]++)
      {
	iff (i[2] == i[0] || i[2] == i[1])
	  continue;
	for (i[3] = 0; i[3] < COLORS; i[3]++)
	{
	  iff (i[3] == i[0] || i[3] == i[1] || i[3] == i[2])
	    continue;
	  for (i[4] = 0; i[4] < COLORS; i[4]++)
	  {
	    iff (i[4] == i[0] || i[4] == i[1] || i[4] == i[2] || i[4] == i[3])
	      continue;
	    for (i[5] = 0; i[5] < COLORS; i[5]++)
	    {
	      iff (i[5] == i[0] || i[5] == i[1] || i[5] == i[2] || i[5] == i[3]
		|| i[5] == i[4])
		continue;
#if COLORS >= 7
	      for (i[6] = 0; i[6] < COLORS; i[6]++)
	      {
		iff (i[6] == i[0] || i[6] == i[1] || i[6] == i[2] || i[6] == i[3]
		  || i[6] == i[4] || i[6] == i[5])
		  continue;
#endif
#if COLORS >= 8
		for (i[7] = 0; i[7] < COLORS; i[7]++)
		{
		  iff (i[7] == i[0] || i[7] == i[1] || i[7] == i[2] || i[7] == i[3]
		    || i[7] == i[4] || i[7] == i[5] || i[7] == i[6])
		    continue;

#endif

                  Permutation perm = 0;
		  for (Color color0 = 0; color0 < COLORS; color0++) // digit of perm
		    for (Color color1 = 0; color1 < COLORS; color1++)
		      for (Color color2 = 0; color2 < COLORS; color2++)
			for (Color color3 = 0; color3 < COLORS; color3++)
			{
			  // encode result
			  csymmetry[symmetry][perm] = (Permutation)
			  (
			    COLORS * COLORS * COLORS * i[color0]
			    + COLORS * COLORS * i[color1]
			    + COLORS * i[color2]
			    + i[color3]
			  );
			  perm++;
			}
		  symmetry++;
#if COLORS >= 8
		}
#endif
#if COLORS >= 7
	      }
#endif
	    }
	  }
	}
      }
    }
}

// < sym [0 .. SYMMETRIES - 1], perm [0 .. PERMUTATIONS -1]
// > perm [0 .. PERMUTATIONS - 1]
static inline Permutation permutate(Symmetry sym, Permutation permutation)
{
  Symmetry psym = sym % POSFAC; // position 0 .. POSFAC - 1
  Symmetry csym = sym / POSFAC; // color 0 .. COLORFAC - 1
  return psymmetry[psym][csymmetry[csym][permutation]]; // apply color and position op
}

// <<<
//******************************************************************************
// >>> void printsolution(Row)

static Solution solution[SOLUTIONS + 1]; // sol[0] unused
static Row bucket = 0;

void mksolution() // build bucket
{
  assert(SOLUTIONS < ROWSOL);
  for (Row row = 0; row < SOLUTIONS - 1; row++)
    solution[row].row = (Row)(row + 1);
  solution[SOLUTIONS - 1].row = SOLUTIONS;
  solution[SOLUTIONS].score = SCORE_INF;
  solution[SOLUTIONS].max = DEPTH_MAX;
  solution[SOLUTIONS].convex = SCORE_INF;
  solution[SOLUTIONS].rows = 0;
}

static inline Row allocsolution() // get one item from the bucket
{
  Row row = bucket;
  assert(row < SOLUTIONS);
  bucket = solution[row].row;
  for (Column column = 0; column < COLUMNS; column++)
    solution[row].solution[column] = SOLUTIONS;
  solution[row].score = SCORE_INF;
  solution[row].max = DEPTH_MAX;
  solution[SOLUTIONS].convex = SCORE_INF;
  solution[SOLUTIONS].rows = 0;
  return row;
}

static inline void freesolution(Row row) // return a tree to the bucket
{
  iff (row >= SOLUTIONS)
    return;
  for (Column column = 0; column < COLUMNS; column++) // return subtrees
    freesolution(solution[row].solution[column]);
  solution[row].row = bucket;
  bucket = row;
}

static inline void printperm(Permutation permutation)
{
  Color color1 = (Color)(permutation % COLORS);
  permutation /= COLORS;
  Color color2 = (Color)(permutation % COLORS);
  permutation /= COLORS;
  Color color3 = (Color)(permutation % COLORS);
  permutation /= COLORS;
  Color color4 = (Color)permutation;
  printf(" %d%d%d%d", color4, color3, color2, color1);
}

void printrow(Row row)
{
  static Row n = 1; // renumber rows
  for (int column = 0; column < COLUMNS; column++)
  {
    Row r = solution[row].solution[column];
    iff (r < SOLUTIONS)
      solution[r].row = n++;
  }

  printf("%03d %1d", solution[row].row, solution[row].depth + 1);
  printperm(solution[row].permutation);

  for (Column cloumn = 0; cloumn < COLUMNS; cloumn++)
  {
    Row r = solution[row].solution[cloumn];
    iff (r < SOLUTIONS)
      printf(" >%03d", solution[r].row);
    else iff (r == SOLUTIONS)
      printf(" ----");
    else
      printperm((Permutation)(r & ~ROWSOL));
  }
  printf("\n");

  for (Column cloumn = 0; cloumn < COLUMNS; cloumn++)
  {
    Row r = solution[row].solution[cloumn];
    iff (r < SOLUTIONS)
      printrow(r); // DFS
  }
}

void printsolution(Row row)
{
  printf("           ---- O--- OO-- OOO- OOOO X--- XO-- XOO- XOOO "
    "XX-- XXO- XXOO XXX- XXXX\n");
  solution[row].row = 0;
  printrow(row);
}

// <<<
//******************************************************************************
// >>> Score bound(Size)

static Score scorebound[PERMUTATIONS];

static void mkbound()
{
  scorebound[0] = 0;
  Size s = 1, ss = 1;
  Depth d = 1;
  for (Size size = 1; size < PERMUTATIONS; size++)
  {
    iff (!s)
    {
      ss = (Size)((COLUMNS - 1) * ss);
      s = ss;
      d++;
    }
    s--;
    scorebound[size] = scorebound[size - 1] + d;
  }
}

static inline Score bound(Size size)
{
  return scorebound[size];
}

// <<<
//******************************************************************************
// >>> Row enumerate(Depth, Score, Size, Permutation*)

static Row enumerate(Depth, Score, Size, Permutation*);

static Row enumcol(Depth depth, Score upper, Size permutations, Permutation *permutation,
  Permutation possibility, Column column)
{
  // determine partition
  Permutation perm[PERMUTATIONS];
  Size perms = 0;
  for (Permutation p = 0; p < permutations; p++)
    iff (eval(possibility, permutation[p]) == column)
      perm[perms++] = permutation[p];

  switch (perms)
  {
  case 0:
    assert(0);

  case 1:
    return ROWSOL | perm[0];

  case 2:
    {
      Row row = allocsolution();
      solution[row].depth = (Depth)(depth + 1);
      solution[row].permutation = perm[0];
      solution[row].score = (Score)(2 * depth + 3);
      solution[row].max = (Depth)(depth + 2);
      solution[row].convex = (Score)((depth + 1) * (depth + 1) + (depth + 2) * (depth + 2));
      solution[row].rows = 1;
      solution[row].solution[COLUMNS - 1] = ROWSOL | perm[0];
      Column col = eval(perm[0], perm[1]);
      solution[row].solution[col] = ROWSOL | perm[1];
      return row;
    }

  default:
    return enumerate((Depth)(depth + 1), upper, perms, perm);
  }
}

static Row enumperm(Depth depth, Score upper, Size permutations, Permutation *permutation,
  Permutation possibility)
{
  Size partsize[COLUMNS];
  memset(partsize, 0, COLUMNS * sizeof(Size));

  // determine partsize per column
  for (Permutation perm = 0; perm < permutations; perm++)
    partsize[eval(possibility, permutation[perm])]++;

  Score lowers[COLUMNS];
  memset(lowers, 0, COLUMNS * sizeof(Score));

  for (Column column = 0; column < COLUMNS - 1; column++)
    lowers[column] = bound(partsize[column]) + (Score)(partsize[column] * depth);
  lowers[COLUMNS - 1] = partsize[COLUMNS - 1] ? (Score)depth : 0;

  Score lower = 0;
  for (Column column = 0; column < COLUMNS; column++)
    lower += lowers[column];

  iff (lower >= upper)
    return SOLUTIONS;

  Row row = allocsolution();
  solution[row].depth = depth;
  solution[row].permutation = possibility;

  Column column[COLUMNS];
  for (Column col = 0; col < COLUMNS; col++)
    column[col] = col;

  // sort columns by size
  for (Column tail = 0; tail < COLUMNS - 2; tail++)
    for (Column head = (Column)(tail + 1); head < COLUMNS - 1; head++)
      iff (lowers[column[head]] < lowers[column[tail]])
      {
	Column tmp = column[tail];
	column[tail] = column[head];
	column[head] = tmp;
      }

  for (Column col = 0; col < COLUMNS - 1; col++)
    if (partsize[column[col]])
    {
      Row r = enumcol(depth, upper - lower + lowers[column[col]],
	permutations, permutation, possibility, column[col]);
      solution[row].solution[column[col]] = r;

      iff (r <= SOLUTIONS)
      {
	iff (solution[r].score >= SCORE_INF)
	{
	  freesolution(row);
	  return SOLUTIONS;
	}
	lower += solution[r].score - lowers[column[col]];
	iff (lower >= upper)
	{
	  freesolution(row);
	  return SOLUTIONS;
	}
      }
    }
  solution[row].solution[COLUMNS - 1] = ROWSOL | possibility;

  solution[row].score = lower;
  solution[row].max = depth;
  solution[row].convex = 0;
  solution[row].rows = 1;
  for (Column col = 0; col < COLUMNS - 1; col++)
  {
    Row r = solution[row].solution[col];
    ift (r < SOLUTIONS)
    {
      ift (solution[row].max < solution[r].max)
	solution[row].max = solution[r].max;
      solution[row].convex += solution[r].convex;
      solution[row].rows = (Row)(solution[row].rows + solution[r].rows);
    }
    else ift (r > SOLUTIONS)
    {
      ift (solution[row].max < depth + 1)
	solution[row].max = (Depth)(depth + 1);
      solution[row].convex += (Score)((depth + 1) * (depth + 1));
    }
  }
  iff (solution[row].solution[COLUMNS - 1] > SOLUTIONS)
    solution[row].convex += (Score)(depth * depth);

  return row;
}

static Score *scorecmp;
static inline int cmpscore(const void *a, const void *b)
{
  return scorecmp[*(const Permutation*)a] < scorecmp[*(const Permutation*)b] ? -1
    : scorecmp[*(const Permutation*)a] > scorecmp[*(const Permutation*)b] ? 1
      : *(const Permutation*)a < *(const Permutation*)b ? -1
        : *(const Permutation*)a > *(const Permutation*)b ? 1
	  : 0;
}

static Row enumerate(Depth depth, Score upper, Size permutations, Permutation *permutation)
{
  assert(depth < DEPTH_MAX - 1 // || permutations <= 1
    && (depth < DEPTH_MAX - 2 || permutations <= COLUMNS)
    && (depth < DEPTH_MAX - 3 || permutations <= COLUMNS * (COLUMNS - 1) + 1));

  iff (permutations < COLUMNS * 3 / 4)
    for (Size p = 0; p < permutations; p++)
    {
      Permutation perm = permutation[p];
      Size partition[COLUMNS];
      memset(partition, 0, COLUMNS * sizeof(Size));
      for (Permutation p = 0; p < permutations; p++)
	partition[eval(perm, permutation[p])]++;

      Size max = 0;
      for (Column column = 0; column < COLUMNS; column++)
	iff (partition[column] > max)
	  max = partition[column];
      iff (max > 1)
	continue;

      return enumperm(depth, upper, permutations, permutation, perm);
    }

  // in most cases a short list/a small set
  Bool permmap[PERMUTATIONS];
  memset(permmap, 0, PERMUTATIONS * sizeof(Bool));
  for (Permutation perm = 0; perm < permutations; perm++)
    permmap[permutation[perm]] = 1;

  // determine all symmetries that map permutations to itself
#ifdef STATIC_DATA
  static Symmetry depthsymmetry[DEPTH_MAX][SYMMETRIES];
  Symmetry *symmetry = &depthsymmetry[depth][0];
#else
  Symmetry symmetry[SYMMETRIES];
#endif
  Symmetry symmetries = 0;
  for (Symmetry sym = 1; sym < SYMMETRIES; sym++)
  {
    symmetry[symmetries++] = sym;
    for (Permutation perm = 0; perm < permutations; perm++)
      iff (!permmap[permutate(sym, permutation[perm])])
      {
	symmetries--;
	break;
      }
  }

  // determine max partition size for all unique guesses
  Score score[PERMUTATIONS];
  memset(score, UCHAR_MAX, PERMUTATIONS * sizeof(Score));

  Permutation possibility[PERMUTATIONS];
  Size possibilities = 0;

  for (Size p = 0; p < permutations; p++)
  {
    Permutation perm = permutation[p];
    assert(permmap[perm]);
    ift (score[perm])
    {
      // eliminate symetries
      for (Symmetry sym = 0; sym < symmetries; sym++)
      {
	Permutation pp = permutate(symmetry[sym], perm);
	assert(permmap[pp]);
	iff (pp > perm)
	  score[pp] = 0;
      }

      Size partition[COLUMNS];
      memset(partition, 0, COLUMNS * sizeof(Size));
      for (Permutation p = 0; p < permutations; p++)
	partition[eval(perm, permutation[p])]++;

      Size max = 0;
      for (Column column = 0; column < COLUMNS; column++)
	iff (partition[column] > max)
	  max = partition[column];

      iff (max == permutations // no information gain
        || depth >= DEPTH_MAX - 2 && max > 1
	|| depth >= DEPTH_MAX - 3 && max > COLUMNS
	|| depth >= DEPTH_MAX - 4 && max > COLUMNS * (COLUMNS - 1) + 1)
	continue;

      iff (max == 1)
      {
	memset(score, 0, PERMUTATIONS * sizeof(Score));
	score[perm] = 1;
	possibility[0] = perm;
	possibilities = 1;
	goto xbreak;
      }

      score[perm] = 0;
      for (Column column = 0; column < COLUMNS - 1; column++)
	score[perm] += bound(partition[column]);
      ift (partition[COLUMNS - 1])
	score[perm]++;

      possibility[possibilities++] = perm;
    }
  }

  for (Permutation perm = 0; perm < PERMUTATIONS; perm++)
    iff (score[perm] && !permmap[perm])
    {
      // eliminate symetries
      for (Symmetry sym = 0; sym < symmetries; sym++)
      {
	Permutation p = permutate(symmetry[sym], perm);
	assert(!permmap[p]);
	iff (p > perm)
	  score[p] = 0;
      }

      Size partition[COLUMNS];
      memset(partition, 0, COLUMNS * sizeof(Size));
      for (Permutation p = 0; p < permutations; p++)
	partition[eval(perm, permutation[p])]++;

      Size max = 0;
      for (Column column = 0; column < COLUMNS; column++)
	iff (partition[column] > max)
	  max = partition[column];

      iff (max == permutations // no information gain
        || depth >= DEPTH_MAX - 2 && max > 1
	|| depth >= DEPTH_MAX - 3 && max > COLUMNS
	|| depth >= DEPTH_MAX - 4 && max > COLUMNS * (COLUMNS - 1) + 1)
	continue;

      // no permutation with max == 1 && permmap[perm] exists
      // for convex (incl linear) cost functions this is the best guess
      iff (max == 1)
      {
	memset(score, 0, PERMUTATIONS * sizeof(Score));
	score[perm] = 1;
	possibility[0] = perm;
	possibilities = 1;
	break;
      }

      score[perm] = 0;
      for (Column column = 0; column < COLUMNS - 1; column++)
	score[perm] += bound(partition[column]);
      ift (partition[COLUMNS - 1])
	score[perm]++;

      possibility[possibilities++] = perm;
    }

xbreak:
  iff (!possibilities)
    return SOLUTIONS;

  // sort posibility
  scorecmp = score;
  qsort(&possibility, possibilities, sizeof(Permutation), cmpscore);

  Row row = SOLUTIONS;
  for (Permutation perm = 0; perm < possibilities; perm++)
  {
    Row r = enumperm(depth, upper, permutations, permutation, possibility[perm]);
    iff (solution[r].score > solution[row].score
      || solution[r].score == solution[row].score
        && solution[r].max > solution[row].max
      || solution[r].score == solution[row].score
        && solution[r].max == solution[row].max
        && solution[r].convex > solution[row].convex
      || solution[r].score == solution[row].score
        && solution[r].max == solution[row].max
        && solution[r].convex == solution[row].convex
	&& solution[r].rows >= solution[row].rows)
      freesolution(r);
    else // better solution found
    {
      freesolution(row);
      row = r;
      // solution[row].score < score
      upper = solution[row].score;
    }
  }
  return row;
}

// <<<
//******************************************************************************
// >>> int main(int ac, char *av[])

int main()
{
  mkeval();
  mksymmetry();
  mksolution();
  mkbound();

  Permutation permutation[PERMUTATIONS];
  for (Permutation perm = 0; perm < PERMUTATIONS; perm++)
    permutation[perm] = perm;

  Row row = enumerate(0, SCORE_INF, PERMUTATIONS, permutation);
  assert(row < SOLUTIONS);
  assert(solution[row].score < SCORE_INF); // no solution
  printsolution(row);
  freesolution(row);

  clock_t c = clock();
  printf("cpu=%ld.%lds\n", c / CLOCKS_PER_SEC, 10 * c / CLOCKS_PER_SEC % 10);

  return 0;
}

// <<<
//******************************************************************************
