# FLUSH+RELOAD

## Notice

This project is unmaintained and somewhat outdated now. We may continue to
answer questions or respond to issues, but not in a timely manner.

[This other project](https://github.com/defuse/flush-reload-attacks) appears
to be an improved implementation of the attack.

## Authors
[Daniel Ge](https://github.com/DanGe42),
[David Mally](https://github.com/djmally),
[Nick Meyer](https://github.com/mkandalf)

## Overview
This is an implementation of the Flush+Reload cache timing side channel attack
as described in a [paper by Yarom and
Falkner](https://eprint.iacr.org/2013/448.pdf). [Our
paper](https://github.com/DanGe42/flush-reload/releases/tag/cis-700-submission)
goes into how we implemented the attack.

In short, Flush+Reload technique exploits *content-based page sharing* (or
*memory deduplication*) to monitor cache lines. To detect whether a program
executes a particular instruction in a given time slot, the technique involves
flushing that portion of instruction memory from cache (using the `clflush` x86
instruction), waiting, and timing how long it takes to access that instruction.
This technique has been demonstrated against the modular exponentiation
procedure in GnuPG pre-1.4.14, which allows an attacker to extract the private
RSA exponent from cache timing data.

## Setting up GPG 1.4.12
As a proof of concept, we attacked GPG 1.4.12. To set this up, there are a few
build scripts in the `built_gpg/` directory. On OS X, run:

```
$ brew install gnupg.rb
```

On Linux (and also on OS X if you don't want to use Homebrew):

```
$ ./install_locally.sh
```

Of course, you should never run arbitrary executable code from the Internet.
You should verify that the scripts do what they are expected to do (they're
pretty short).

## Running our code
Run `make` first.

To make things easier, we have provided a convenience script `run.sh`. Edit the
relevant variables in the file before running it.

To obtain addresses to target, the simplest way to begin is to open the
executable in GDB or LLDB and set breakpoints. You can take a look at some of
our sample annotated address txt files in the `docs/addr/` directory to give
you a good starting point for which GPG source lines to target. Some files
contain 3 addresses and some files contain 6, which could, understandably, be
confusing. These are artifacts of the various trials we ran.  Ultimately, our
successful attack on EC2 used only three probes, but feel free to experiment.

Also, note that the annotated EC2 file does not have line numbers. We ended up
digging into the disassembly to find those addresses, which, in that case, were
ones corresponding to addresses close to the `retq` instruction for those
functions. More details are in our linked implementation paper.

## More papers to read
As a result of our unexpected Google search ranking for "flush+reload", we have
seen a slight uptick of interest in our code (just a couple of emails, really).
While we can't answer every question, we do have suggestions for papers worth
reading for anyone who is looking to apply/extend the principles of the
FLUSH+RELOAD attack:

* [Wait a minute! A fast, Cross-VM attack on AES](https://eprint.iacr.org/2014/435.pdf)
* [Seriously, get off my cloud! Cross-VM RSA Key Recovery in a Public Cloud](https://eprint.iacr.org/2015/898.pdf)
* [Cross Processor Cache Attacks](https://eprint.iacr.org/2015/1155.pdf)
* [Cache Template Attacks: Automating Attacks on Inclusive Last-Level
  Caches](https://www.usenix.org/system/files/conference/usenixsecurity15/sec15-paper-gruss.pdf)
  (Suggested by a passing university student who asked us questions about our code)
* [Make Sure DSA Signing Exponentiations Really are Constant-Time](https://eprint.iacr.org/2016/594.pdf)
  (A recent OpenSSL DSA key recovery attack co-authored by Yarom)
