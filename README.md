# DQRAT-check

## Architecture

List of classes; for each class its public API that it should expose to other classes.
Methods called `notifyX` should be called by other classes when they make a change of which another class should be notified.

- `ProofChecker` class to orchestrate the actual checking process, and to perform the checks using the other manager classes
- `ProofReader` class to read in proof files / streams
    - should return a `Proof` object on which a `ProofChecker` can operate
- `PrefixManager` class to maintain data about the prefix
    - methods should return if they succeeded
    - `bool addVar(VarSet D, Var x, bool existential)`
    - `bool deleteVar(Var x)`
- `OuterManager` class to answer queries about which variables are outer / provide lists of outer variables if necessary
    - `void notifyVarAdded(Var x)`
    - `void notifyVarDeleted(Var x)`
    - `void notifyDependencyRemoved(Var u, Var x)`
    - `bool isOuter(Var x, Var y)`
- `DependencySchemeManager` class to answer queries about independence according to D-∀pure
    - `void notifyClauseAdded(int64_t clause_id)`
    - `bool dependsOn(Var x, Var u)`
    - `bool listDependsOn(Var u)`
- `ClauseDatabase` class to store, add, and delete clauses, and also to perform unit propagation
    - `addClause(C)`
    - `deleteClause(C)`
    - (maybe: `modifyClause(C)` for universal reduction)
