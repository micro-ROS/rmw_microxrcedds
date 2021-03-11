This document is a declaration of software quality for **rmw_microxrcedds_c** based on the guidelines provided in the [ROS 2 REP-2004 document](https://www.ros.org/reps/rep-2004.html).

# Quality Declaration

**rmw_microxrcedds_c** is a RMW implementation for ROS and micro-ROS that uses [eProsima Micro XRCE-DDS](https://github.com/eProsima/Micro-XRCE-DDS) as middleware library.

**rmw_microxrcedds_c** claims to be in the **Quality Level 2** category.

Below are the rationales, notes and caveats for this claim, organized by each requirement listed in the [Package Requirements for Quality Level 2 in REP-2004](https://www.ros.org/reps/rep-2004.html#package-requirements).

## Version Policy [1]

### Version Scheme [1.i]

The **Versioning Policy Declaration** for **rmw_microxrcedds_c** can be found [here](VERSIONING.md) and it adheres to [`semver`](https://semver.org/).

### Version Stability [1.ii]

**rmw_microxrcedds_c** is at a stable version, i.e. `>=1.0.0`.
The latest version and the release notes can be found [here](https://github.com/micro-ROS/rmw-microxrcedds/releases).

### Public API Declaration [1.iii]

All symbols in the installed headers are considered part of the public API.

All installed headers are in the include directory of the package, headers in any other folders are not installed and considered private.

### API Stability Within a Released ROS Distribution [1.iv]/[1.vi]

**rmw_microxrcedds_c** will not break public API within a released ROS distribution, i.e. no major releases once the ROS distribution is released.

### ABI Stability Within a Released ROS Distribution [1.v]/[1.vi]

**rmw_microxrcedds_c**  contains C and C++ code and therefore must be concerned with ABI stability, and will maintain ABI stability within a ROS distribution.
Any ABI break in **rmw_microxrcedds_c** will be done between minor versions and it should be clearly stated on the release notes.

## Change Control Process [2]

The stability of **rmw_microxrcedds_c** is ensured through reviews, CI and tests.
The change control process can be found in [CONTRIBUTING](CONTRIBUTING.md)

All changes to **rmw_microxrcedds_c** occur through pull requests that are required to pass all CI tests.
In case of failure, only maintainers can merge the pull request, and only when there is enough evidence that the failure is unrelated to the change.
Additionally, all pull requests must have a positive review from one other contributor that did not author the pull request.

### Change Requests [2.i]

All changes will occur through a pull request.

### Contributor Origin [2.ii]

**rmw_microxrcedds_c** uses the [Developer Certificate of Origin (DCO)](https://developercertificate.org/) as its confirmation of contributor origin policy since version 1.2.2.
More information can be found in [CONTRIBUTING](CONTRIBUTING.md)

### Peer Review Policy [2.iii]

All pull requests will be peer-reviewed by at least one other contributor who did not author the pull request. Approval is required before merging.

### Continuous Integration [2.iv]

All pull requests must pass CI to be considered for merging, unless maintainers consider that there is enough evidence that the failure is unrelated to the changes.
CI testing is automatically triggered by incoming pull requests.
Current results can be seen here:

* [![CI rmw_microxrcedds_c](https://github.com/micro-ROS/rmw-microxrcedds/actions/workflows/ci.yml/badge.svg?branch=foxy&event=push)](https://github.com/micro-ROS/rmw-microxrcedds/actions/workflows/ci.yml)
* 
## Documentation [3]

### Feature Documentation [3.i]

TODO HERE

### License [3.iii]

The license for **rmw_microxrcedds_c** is Apache 2.0, and a summary can be found in each source file.
A full copy of the license can be found [here](LICENSE).

### Copyright Statements [3.iv]

**rmw_microxrcedds_c** copyright holder provide a statement of copyright in each source file.

## Testing [4]

### Feature Testing [4.i]

**rmw_microxrcedds_c** provides tests which simulate typical usage, and they are located in the [`test` directory](test).
New features are required to have tests before being added as stated in [CONTRIBUTING](CONTRIBUTING.md).
Current results can be found here:

* [![CI rmw_microxrcedds_c](https://github.com/micro-ROS/rmw-microxrcedds/actions/workflows/ci.yml/badge.svg?branch=foxy&event=push)](https://github.com/micro-ROS/rmw-microxrcedds/actions/workflows/ci.yml)

### Coverage [4.iii]

TODO
<!-- 
[![Coverage](https://img.shields.io/jenkins/coverage/cobertura?jobUrl=http%3A%2F%2Fjenkins.eprosima.com%3A8080%2Fview%2FMicro%2520XRCE%2Fjob%2FMicro-CDR%2520Nightly%2520Master%2520Linux%2F)](http://jenkins.eprosima.com:8080/view/Micro%20XRCE/job/Micro-CDR%20Nightly%20Master%20Linux/)
*rmw_microxrcedds_c* aims to provide a line coverage **above 90%**.
*Micro CDR* code coverage policy comprises:
1. All contributions to *Micro CDR* must increase (or at least keep) current line coverage.
   This is done to ensure that the **90%** line coverage goal is eventually met.
1. Line coverage regressions are only permitted if properly justified and accepted by maintainers.
1. If the CI system reports a coverage regression after a pull request has been merged, the maintainers must study the case and decide how to proceed, mostly reverting the changes and asking for a more thorough testing of the committed changes.
2. This policy is enforced through the [nightly Micro CDR CI job](http://jenkins.eprosima.com:8080/view/Micro%20XRCE/job/Micro-CDR%20Nightly%20Master%20Linux/).

As stated in [CONTRIBUTING.md](CONTRIBUTING.md), developers and contributors are asked to run a line coverage assessment locally before submitting a PR. -->
### Linters and Static Analysis [4.v]

**rmw_microxrcedds_c** [code style](https://github.com/eProsima/cpp-style) is enforced using [*uncrustify*](https://github.com/uncrustify/uncrustify).
Among the CI tests there are tests that ensures that every pull request is compliant with the code style.
The latest CI results can be seen [here](https://github.com/micro-ROS/rmw-microxrcedds/actions/workflows/ci.yml).

TODO: clang tidy for static analisys.
TODO: https://github.com/ament/ament_lint/blob/master/ament_lint_common/doc/index.rst

## Dependencies [5]

### Direct Runtime Dependencies [5.iii]

**rmw_microxrcedds_c**  depends on the following packages:
* `eProsima Micro XRCE-DDS-Client`: [QUALITY DECLARATION](https://github.com/eProsima/Micro-XRCE-DDS-Client/blob/master/QUALITY.md)
* `eProsima Micro CDR`: [QUALITY DECLARATION](https://github.com/eProsima/Micro-CDR/blob/master/QUALITY.md)
* `rcutils`: [QUALITY DECLARATION](https://github.com/ros2/rcutils/blob/master/QUALITY_DECLARATION.md)
* `rmw`: [QUALITY DECLARATION](https://github.com/ros2/rmw/blob/master/rmw/QUALITY_DECLARATION.md)
* `rosidl_runtime_c`: [QUALITY DECLARATION](https://github.com/ros2/rosidl/blob/master/rosidl_runtime_c/QUALITY_DECLARATION.md)
* `rosidl_typesupport_microxrcedds_c`: TODO

## Platform Support [6]

**rmw_microxrcedds_c** supports the all the platforms that are intended for. Those included GNU/Linux and POSIX systems and all the major embedded RTOS such as Zephyr RTOS, FreeRTOS and Nuttx. Windows is not included as Tier 1 platform because it is irrelevant for the nature of this package.

More information about the supported platforms can be found in [PLATFORM_SUPPORT](PLATFORM_SUPPORT.md)

## Vulnerability Disclosure Policy [7.i]

**rmw_microxrcedds_c** vulnerability Disclosure Policy can be found [here](https://github.com/eProsima/policies/blob/main/VULNERABILITY.md)

# Current Status Summary

The chart below compares the requirements in the [REP-2004](https://www.ros.org/reps/rep-2004.html#quality-level-comparison-chart) with the current state of **rmw_microxrcedds_c**
| Number  | Requirement                                       | Current State |
| ------- | ------------------------------------------------- | ------------- |
| 1       | **Version policy**                                | ---           |
| 1.i     | Version Policy available                          | ✓             |
| 1.ii    | Stable version                                    | ✓             |
| 1.iii   | Declared public API                               | ✓             |
| 1.iv    | API stability policy                              | ✓             |
| 1.v     | ABI stability policy                              | ✓             |
| 1.v_    | API/ABI stable within ros distribution            | ✓             |
| 2       | **Change control process**                        | ---           |
| 2.i     | All changes occur on change request               | ✓             |
| 2.ii    | Contributor origin (DCO, CLA, etc)                | ✓             |
| 2.iii   | Peer review policy                                | ✓             |
| 2.iv    | CI policy for change requests                     | ✓             |
| 3       | **Documentation**                                 | ---           |
| 3.i     | Per feature documentation                         | TODO          |
| 3.iii   | Declared License(s)                               | ✓             |
| 3.iv    | Copyright in source files                         | ✓             |
| 3.v.a   | Quality declaration linked to README              | ✓             |
| 3.v.b   | Centralized declaration available for peer review | ✓             |
| 4       | **Testing**                                       | ---           |
| 4.i     | Feature items tests                               | ✓             |
| 4.iii.a | Using coverage                                    | TODO          |
| 4.v.a   | Code style enforcement (linters)                  | TODO          |
| 4.v.b   | Use of static analysis tools                      | TODO          |
| 5       | **Dependencies**                                  | ---           |
| 5.iii   | Justifies quality use of dependencies             | ✓             |
| 6       | **Platform support**                              | ---           |
| 6.i     | Support targets Tier1 ROS platforms               | ✓             |
| 7       | **Security**                                      | ---           |
| 7.i     | Vulnerability Disclosure Policy                   | ✓             |