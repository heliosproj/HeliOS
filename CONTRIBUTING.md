# Contributing to HeliOS
We love that you are interested in joining the HeliOS Project community as a contributor. We want to make contributing to the project as easy and seamless as possible, whether it is:

- Reporting a bug
- Submitting a bug fix
- Asking a question
- Proposing new features
- Submitting a new feature
- Improving documentation
- Becoming a maintainer

## We Develop with Github
The HeliOS Project uses GitHub exclusively to maintain and develop HeliOS. The best methods for engaging with the HeliOS Project is through GitHub Issues and Pull Requests.

Submit a:
- **Issue**: If you are reporting a bug, asking a question or proposing a new feature, then submit an issue. Please do not submit source code or other file changes through an issue. If you do, you will be asked to submit a pull request and your issue will be closed. See the **Submitting an Issue** section below for more details on submitting an issue.
- **Pull Request**: If you are submitting a bug fix, a new feature or a documentation change, then submit a pull request. See the **Submitting a Pull Request** section below for more details.

## Submitting an Issue

If you are reporting a bug then submit a **Bug Report** issue. If you are in need of help or seeking support then submit a **Support Request**. Anything else should be submitted as an **Other Request**. To open a new issue, go [here](https://github.com/heliosproj/HeliOS/issues). We kindly ask for your patience as it may take several days before we are able to respond to your issue. If it is a bug report or support request, it may take longer.

## Submitting a Pull Request

First and foremost, it may take a week or more before your pull request is merged (if at all). All pull requests are reviewed multiple times and we may request you make changes before your pull request is merged. Once your pull request is merged, it will not appear in the mainline HeliOS source code until the next release which may be months away. To submit a pull request, the following steps **must** be followed:

1. Fork the repository (you must uncheck **[x] Copy the ``main`` branch only.** option before forking the repository) into your personal GitHub account. If you do not perform this step, only the ``master`` branch will be forked.
2. Create a new branch named ``develop-<account_name>`` from the ``develop``. **Do not make your changes to anything in the ``master`` branch which contains the latest release. If you do, your pull request will later be rejected.**
3. Make your source code changes, create a commit and push to the remote (GitHub). 
4. If you have added a new feature, you **must** add unit tests for the new feature. **Pull requests for new features not accompanied by unit tests will be rejected.**
5. Your code changes must include code comments (where appropriate) and be accompanied by documentation updates (again, where appropriate). Most documentation changes are made to the doxygen documentation in ``/src/HeliOS.h`` which is used to generate the HeliOS Developer's Guide.
6. Ensure your code compiles without warnings and the unit tests pass.
5. Run Uncrustify against the source code files. The HeliOS Uncrustify config is available as ``/extras/uncrustify/helios.cfg``.
8. Issue that pull request and hang tight! :)

## Copyright & License
In short, when you contribute source code, documentation or other file changes to the HeliOS project, those contributions will become part of the HeliOS Project license and copyright.

## Thank You
Thank you for your contributions to HeliOS.