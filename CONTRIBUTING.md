# How to contribute to Robust Services Core

Thanks for your interest!  This document describes different ways that you can contribute.

## Asking a question
Questions are helpful because they can identify where documentation needs to be improved.
If you can't find the answer there, send your question to Greg Utas (greg@pentennea.com).

## Collaboration
To contribute in other ways, you need to be a collaborator.  Send an email to Greg Utas
(greg@pentennea.com) to be invited to the repository.

## Issues

The other ways to contribute involve the use of GitHub issues, which are labelled as follows:

Label | Meaning
------|--------
`application` | focuses on software in the `mb`, `cb`, `pots`, `sn`, `an`, `rn`, or `on` directory
`blocked` | another item needs to be completed before this one
`bug` | a problem that needs to be fixed
`core` | focuses on software in the `nb`, `nw`, `sb`, `cn`, or `rsc` directory
`core tools` | focuses on software in the `nt` or `st` directory
`c++ tools` | focuses on software in the `ct` or `subs` directory
`design` | discussed in [_RSC Software Design_](/docs/RSC-Software-Design.pdf)
`documentation` | additions or changes to documents or code comments
`duplicate` | already described in another issue
`enhancement` | a capability that should be added
`help wanted` | an issue where collaboration is especially sought
`invalid/declined` | will not be implemented
`question` | something that should be added to a FAQ
`1` | a day's effort once reasonably familiar with the software
`2` | several day's effort once reasonably familiar with the software
`3` | around a week's effort once reasonably familiar with the software
`4` | several week's effort once reasonbly familiar with the software
`5` | a major project

If you're a new contributor, you may want to start with an item labelled `1` or `2`.

When creating an issue
- Choose a descriptive title and appropriate labels.
- If you plan to work on the issue, provide enough detail so that others can
understand what you plan to achieve.
- If you don't plan to work on the issue, provide enough detail so that what
gets implemented will be what you wanted.

## Reporting a bug
Describe what you did and how the results differed from what you expected.  Attach a
console file and log file.  If possible, also attach a test script that can be `>read`
from the console, along with the output from a trace tool.

## Suggesting an enhancement
Describe the expected behavior.  Unless it is obvious why the new capability would be
beneficial, include a rationale and/or examples of how it would be used.

## Writing code
Before working on code
- Make sure that the change has been documented as an issue.
- Ask to be assigned to the issue so that other contributors can see what is already
being worked on.
- Read the [coding guidelines](/docs/RSC-Coding-Guidelines.md) and the [second
chapter](/docs/RCS-chapter-2.pdf) of *Robust Communications Software*.