# rttm
Relative Timed Trace Manager

This software was developed in order to translate and compare relative timed traces.
Synopsis:
```
rttm <MODE> <options>
```

Two modes are available -T for translation and -V for Verification
(try the command ./rttm or ./rttm --help for more details).

-- --
# Getting Started

## Prerequisites
* [Maven](https://maven.apache.org), it is usually available in the package manager of your distribution.

## Clone and Compile
Clone the repo and compile the sources:
```sh
git clone https://github.com/cponcelets/rttm.git
cd rttm
mvn compile
```

# Example
Tests examples will be created for a first step to understand the tool.
We plan a documentation to describe input/output trace formats, see the Clement Poncelet Sanchez page to have more publications (http://mypage.work).

The following script launches the tests benchmark.
```sh
mvn test
```
