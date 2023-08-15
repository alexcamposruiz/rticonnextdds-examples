# Generates a markdown table with a summary of the examples in the repository.
# The examples have the following structure:
# examples/connext_dds/<example_name>/README.md
# examples/connext_dds/<example_name>/<language>
#
# This script scans each example for its README, extracts its description, then
# it determines which languages are supported. With this information it creates
# a table with the following columns: Example, Description, C, C++98, C++11, Python, Java, C#.

import os
import re

LANGS = ["c", "c++98", "c++11", "py", "java", "cs"]

def generate_example_table():
    # Initialize table headers
    table = "| Example | Description | C | C++98 | C++11 | Python | Java | C# |\n"
    table += "| ------- | ----------- | - | ----- | ----- | ------ | ---- | -- |\n"

    # Find all example directories
    example_dir = "examples/connext_dds"
    examples = [f.path for f in os.scandir(example_dir) if f.is_dir()]

    # sort alphabetically
    examples.sort()

    # Loop through each example
    for example in examples:
        # Find README file
        readme_file = os.path.join(example, "README.md")
        if not os.path.isfile(readme_file):
            continue

        # Extract description from README
        with open(readme_file, "r") as f:
            readme = f.read()
        description = re.search(r"## Concept\n\n(.+?)\n\n", readme, re.DOTALL)
        if not description:
            continue
        description = description.group(1).replace('\n', ' ').strip()

        # Extract title from README
        title = re.search(r"# (.+)\n", readme)
        if not title:
            continue
        title = title.group(1).replace('Example Code:', '').strip()
        title = f'**{title}**'

        # Extract directory name
        directory_name = os.path.basename(example)

        # Create link to directory
        directory_link = f"[(...)]({directory_name})"

        # Concatenate description and link
        description = f"{title}: {description} {directory_link}"

        # Find supported languages
        languages = [f.name for f in os.scandir(example) if f.is_dir()]

        # Add row to table
        if not any(lang in languages for lang in LANGS):
            # The directory likely contains subdirectories with examples in them
            row = f"| [{directory_name}]({directory_name}) | {description} |  |"
        else:
            row = f"| [{directory_name}]({directory_name}) | {description} |"
            for lang in LANGS:
                if lang == "cs" and "Legacy .NET API" in readme:
                    row += "   |"
                elif lang in languages:
                    row += " ✓ |"
                else:
                    row += "   |"
        row += "\n"
        table += row

    return table

print(\
"""
# Connext DDS

RTI Connext DDS is a software connectivity framework for real-time distributed
applications. It provides the middleware communications service that
programmers need to distribute time-critical data between embedded and/or
enterprise devices or nodes. Connext DDS uses a publish-subscribe
communications model to make data-distribution efficient and robust. Connext
DDS also supports the Request-Reply communication pattern.

In this directory you will find examples that illustrate some of the most
important features of the RTI Connext DDS core libraries. For documentation,
including the Getting Started Guide, User's Manual, Release Notes, and more see
https://community.rti.com/documentation.

## Examples
"""\
)
print(generate_example_table())