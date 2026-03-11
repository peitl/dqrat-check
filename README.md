# DQRAT-check

DQRAT-check is a checker for proofs in the DQRAT+$`\mathcal{D}^{\forall \text{pure}}`$ format, which builds on the DQRAT format and extends it with a powerful new dependency scheme.
A paper for DQRAT-check is currently under submission to SAT 2026.

## Install

```bash
git clone https://github.com/peitl/dqrat-check
cmake -Bbuild
cmake --build build
```

optionally followed by `cmake --install build` (will put `dqrat-check` on path).

## Usage

`dqrat-check <formula.dqdimcas> <proof.dqrat>` 
