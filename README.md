dynQBF (version 0.4.0) 
====================

dynQBF is a structure-aware QBF solver. It handles QBF instances
in prenex CNF format, and supports QSAT (deciding satisfiability
of QBFs) as well as enumerating solutions (partial certificates
for the outermost quantifier block). In a nutshell, dynQBF splits
the QBF instance into subproblems by constructing a so-called
tree decomposition. The QBF is then solved by dynamic programming
over the tree decomposition. As key ingredient, dynQBF uses
Binary Decision Diagrams to efficiently store intermediate results.

Contact 
-------

Contact:    Guenther Charwat: gcharwat at dbai dot tuwien dot ac dot at

WWW:        http://dbai.tuwien.ac.at/proj/decodyn/dynqbf/

Source:     https://github.com/gcharwat/dynqbf

Version info 
--------------

2016-12-19: dynQBF 0.4.1
- Fixed unnecessary iteration in unsatisfiability check
- Compatibility with htd release 1.0.1
(see https://github.com/mabseher/htd/releases/tag/1.0.1)

2016-12-16: dynQBF 0.4.0
- Major refactoring of the source code, yielding significantly better performance. Changes include
..* remove cache now supports better balancing of NSF and BDD sizes
..* improved global NSF size estimation
..* tuning of removal and subset checks
- Unsatisfiability checks can now be controlled via command line
- Per default, large clauses are splitted in a preprocessing step
- Added new root and decomposition selection strategies
- Git version of dynqbf and htd can now be printed via option -v
- Reworked command line interface options (description, default values, etc)
- Compatibility with htd release 1.0.0
(see https://github.com/mabseher/htd/releases/tag/1.0.0)

2016-11-10: dynQBF 0.3
- Heuristic TD and TD root node selection with various fitness functions, see options
   --root-strategy <f> 
   --decomposition-strategy <f>
- Added an experimental implementation for a 2QBF solver that
  uses only a single BDD, see option 
   -p cnf2
- This version adds initial support for preprocessing 
  - conversion to 3CNF
  - unit literals
- Improved option handling, refactored instance handling
- Improved subset checking (should improve performance)
- Compatibility and tested with HTD release htd 1.0.0 (beta1) 
  (see https://github.com/mabseher/htd/releases/tag/v1.0.0-beta1)

2016-05-10: dynQBF 0.2
 - Bug fix for QBFs with more than 2 quantifiers
 - Added dynamic variable reordering for BDDs
 - Refined support for BDD size <-> NSF size balancing
 - Activated min-fill heuristic for TD generation
 - Various performance improvements (NSF manager, updated HTD lib)

2016-02-24: dynQBF 0.1-beta
 - Currently only available as statically linked x86-86 binary

License
-------

Released under the GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
A copy of the license should be provided with the system, otherwise see
http://www.gnu.org/licenses/

Building dynQBF 
---------------

For instructions about compiling, please read the INSTALL file.

Running dynQBF
--------------

A simple program call is of the following form:

    ./dynqbf < $file

Run `./dynqbf -h` to get a complete list of all available options.
In order to get reproducible results, you might want to fix the seed by using option `--seed <s>`. 
