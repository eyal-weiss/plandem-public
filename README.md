# PlanDEM

PlanDEM is a research project, developed at Bar-Ilan University,
that aims to build a domain-independent classical planning system
which uses dynamically estimated action models.
It is based on the Fast Downward planning system,
with modifications that support dynamic action model estimatation.

Copyright 2021--2023 PlanDEM contributors (see below).

For further information:
- PlanDEM main repository: <https://github.com/eyal-weiss/plandem-public>
- Fast Downward website: <http://www.fast-downward.org>


## Tested software versions

This version of PlanDEM is tested with the following software versions:

| OS           | Python | C++ compiler                                                     | CMake |
| ------------ | ------ | ---------------------------------------------------------------- | ----- |
| Ubuntu 20.04 | 3.8    | GCC 9, GCC 10, Clang 10, Clang 11                                | 3.16  |


## Contributors

The following list includes all people that actively contributed to PlanDEM.

- 2021--2023 Eyal Weiss
- 2021--2023 Gal A. Kaminka

## Papers

- Planning with Multiple Action-Cost Estimates, Eyal Weiss and Gal A. Kaminka, ICAPS 2023
- A Generalization of the Shortest Path Problem to Graphs with Multiple Edge-Cost Estimates, Eyal Weiss, Ariel Felner and Gal A. Kaminka, ICAPS RDDPS Workshop 2023

Relevant information appears in directories with the same name.

## Build

Same as Fast Downward.

## Run

Same as Fast Downward, but with the following choices in the run command:
- To run ACE choose the search engine "synchronic". See documentation in plugin_synchronic_estimation.cc. To switch between estimator types, open the file synchronic_estimation_search.cc and modify the class of *estimator_ptr (currently two options: Estimator or OntarioEstimator) and the input parameters of get_estimator accordingly.
- To run BEAUTY choose the search engine "beauty". See documentation in plugin_beauty.cc. To run Anytime-BEAUTY choose the search engine "anytime_beauty". See documentation in anytime_beauty.cc.

## License

The following directory is not part of PlanDEM as covered by this license:

- ./src/search/ext

For the rest, the following license applies:

```
PlanDEM is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

PlanDEM is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
```
