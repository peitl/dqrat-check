# DQRAT-check

DQRAT-check is a checker for proofs in the DQRAT+$`\mathcal{D}^{\forall \text{pure}}`$ format, which builds on the DQRAT format and extends it with a powerful new dependency scheme.
A paper for DQRAT-check is currently under submission to SAT 2026.
At the moment, these instructions are intended for reviewers inspecting the supplementary material.
They will be extended upon publication.

## Install

```bash
git clone https://github.com/peitl/dqrat-check
cmake -Bbuild
cmake --build build
```

optionally followed by `cmake --install build` (will put `dqrat-check` on path, may require `sudo` or modifying [CMAKE_INSTALL_PREFIX](https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html)).
The compiled `dqrat-check` binary is at `build/src/dqrat-check`.

## Usage

`dqrat-check <formula.dqdimcas> <proof.dqrat>` 
