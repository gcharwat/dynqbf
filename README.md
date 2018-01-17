dynQBF 
====================

dynQBF is an expansion-based QBF solver for instances in prenex CNF form.  First, 
the CNF matix is split into subproblems by constructing a tree decomposition. 
The QBF is then solved by dynamic programming over the tree decomposition. While 
the structure of the CNF is reflected by the tree decomposition, structure within 
the prefix is considered by integrating dependency schemes in the solving process.
Furthermore, dynQBF uses nested sets of binary decision diagrams  (BDDs) to 
efficiently store intermediate results.

The core concepts of dynQBF are described in [1] and [2]. The solver relies on 
htd (https://github.com/mabseher/htd) for constructing the tree decomposition 
and CUDD (http://vlsi.colorado.edu/~fabio/) for handling the BDDs. Optionally, 
DepQBF (https://github.com/lonsing/depqbf) can be used to compute the standard 
dependency scheme.

[1] G. Charwat and S. Woltran. BDD-based dynamic programming on tree decompositions. 
  Technical Report DBAI-TR-2016-95, TU Wien, 2016.

  http://www.dbai.tuwien.ac.at/research/report/dbai-tr-2016-95.pdf

[2] G. Charwat and S. Woltran. Dynamic programming-based QBF solving. In QBF@SAT, 
volume 1719 of CEUR Workshop Proceedings, pages 27–40, 2016.

  http://ceur-ws.org/Vol-1719/paper2.pdf


Contact 
-------

Contact:    Guenther Charwat: gcharwat at dbai dot tuwien dot ac dot at

WWW:        http://dbai.tuwien.ac.at/proj/decodyn/dynqbf/

Source:     https://github.com/gcharwat/dynqbf


Building dynQBF 
---------------

For instructions about compiling, please read the INSTALL file.

Precompiled binaries are available at https://github.com/gcharwat/dynqbf/releases

Running dynQBF
--------------

A simple program call is of the following form:

    ./dynqbf -f $file

Run `./dynqbf -h` to get a complete list of all available options.
In order to obtain reproducible results, you might want to fix the seed by using option `--seed <s>`. 


License
-------

Released under the GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
A copy of the license should be provided with the system, otherwise see
http://www.gnu.org/licenses/


Version info 
--------------

2018-01-17: dynQBF 1.1.1
- Fixing version information and README

2018-01-17: dynQBF 1.1.0
- Counting support
- Enumeration: limit number of sum-over-product cover models

2017-07-24: dynQBF 1.0.2
- Optimization intervals

2017-07-18: dynQBF 1.0.1
- Fixed a bug related to QDIMACS input files with \r\n line ending
- Per default, now 30 tree decompositions are generated for TD selection
- Order of quantification and conjunction/disjunction changed  
- htd graph preprocessing options (--td-preprocessing)
- Added enhanced maximum cardinality search
- Compatibility with htd release 1.2.0
(see https://github.com/mabseher/htd/releases/tag/1.2)

2017-06-25: dynQBF 1.0.0-final
- Default parameter tuning

2017-04-18: dynQBF 1.0.0-rc.6
- Improved NSF/BDD size balancing
- Minor TD fitness function optimizations
- New LeafNodeCount fitness function
- Dynamic TD selection strategy is now available (2-QBFs: join-child-count, removed-level otherwise)
- Printing License information is now supported
- Compatibility with htd release 1.1-rc1-bugfix 
(see https://github.com/mabseher/htd/releases/tag/1.1-rc1-bugfix)

2017-03-10: dynQBF 1.0.0-rc.1
- Dynamic dependency scheme selection based on quantifier prefix
- Added detailed decomposition and NSF statistics output
- Unsatisfiable parts of NSFs ore now truncated
- Integrated tree decomposition preprocessing
- Removal cache can now be disabled
- Memory improvements: instance parsing and representation is now more efficient
- Improvements w.r.t. solution enumeration
- Fixed a bug when compiled with clang
- Compatibility with htd release 1.1-rc1
(see https://github.com/mabseher/htd/releases/tag/1.1-rc1)

2017-01-19: dynQBF 0.5.1
- Fixed a bug related to the default setting of the selected decomposition strategy
- Minor code cleaning

2017-01-17: dynQBF 0.5.0
- Added support for dependency schemes (none, simple and standard)
- "standard" is computed by DepQBF, its integration can be deactivated by modifying the Makefile, see the INSTALL file
- Added RemovedLevelFitness function (that should work well in combination with the "standard" and "simple" dependency schemes)
- Implemented signal handling: Benchmark information is now printed before exit
- The new "verbose" printer prints all computation steps, including the computed BDDs
- The "debug" printer now only prints the size of the BDDs in the NSF data structure

2016-12-19: dynQBF 0.4.1
- Fixed unnecessary iteration in unsatisfiability check
- Compatibility with htd release 1.0.1
(see https://github.com/mabseher/htd/releases/tag/1.0.1)

2016-12-16: dynQBF 0.4.0
- Major refactoring of the source code, yielding significantly better performance. Changes include
  * remove cache now supports better balancing of NSF and BDD sizes
  * improved global NSF size estimation
  * tuning of removal and subset checks
- Unsatisfiability checks can now be controlled via command line
- Per default, large clauses are splitted in a preprocessing step
- Added new root and decomposition selection strategies
- Git version of dynqbf and htd can now be printed via option -v
- Reworked command line interface options (description, default values, etc)
- Compatibility with htd release 1.0.0
(see https://github.com/mabseher/htd/releases/tag/1.0.0)

2016-11-10: dynQBF 0.3
- Heuristic TD and TD root node selection with various fitness functions, see options for
  * root strategy (iterations)
  * decomposition strategy (iterations)
- Added an experimental implementation for a 2QBF solver that
  uses only a single BDD
- This version adds initial support for preprocessing 
  * conversion to 3CNF
  * unit literals
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

