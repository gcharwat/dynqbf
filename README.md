dynQBF (version 0.2) 
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

2016-05-10: dynQBF 0.2
 - Bug fix for QBFs with more than 2 quantifiers
 - Added dynamic variable reordering for BDDs
 - Refined support for BDD size <-> NSF size balancing
 - Activated min-fill heuristic for TD generation
 - Various performance improvements (NSF manager, updated HTD lib)

2016-02-24: dynQBF 0.1-beta
 Currently only available as statically linked x86-86 binary

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

    dynqbf --reorder lazy-sift --opt-interval 4 --max-NSF-size 1000 --max-BDD-size 3000 --check-unsat < $file

where (some of the available interesting) optimizations are configured by 
    --reorder <h>          : Use dynamic BDD variable reordering heuristic <h>
    --opt-interval <i>     : Optimize NSF every <i>-th computation step (default: 4, disable: 0)
    --max-NSF-size <s>     : Split until NSF size <s> is reached (Recommended: 1000)
    --max-BDD-size <s>     : Always split if a BDD size exceeds <s> (Recommended: 3000, overrules max-NSF-size)
    --sort-before-joining  : Sort NSFs by increasing size before joining; can increase subset check success rate
    --check-unsat          : Check for unsatisfiable computations

Additional (interesting) options:

    --enumerate		enumerate all minterms (outermost, existential quantifier, if instance is SAT)
    --seed <s>		fix seed to obtain reproducable benchmark results
    -o <ordering>		configure variable ordering in BDDs (min-cut, max-bag and max-clause are currently experimental)
    --output <module>      quiet, progress (more info), debug (most info)
    -h			see the help for a complete list of all available options
