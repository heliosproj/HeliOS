#!/usr/bin/env python3
"""
HeliOS Coding Standard Rule Review Tool

This interactive tool allows you to review each coding standard rule,
mark rules that need revision, and add comments for later updates.
"""

import json
import os
import sys
from typing import Dict, List, Optional


class Colors:
    """ANSI color codes for terminal output"""
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    END = '\033[0m'

    @staticmethod
    def disable():
        """Disable colors (for non-ANSI terminals)"""
        Colors.HEADER = ''
        Colors.BLUE = ''
        Colors.CYAN = ''
        Colors.GREEN = ''
        Colors.YELLOW = ''
        Colors.RED = ''
        Colors.BOLD = ''
        Colors.UNDERLINE = ''
        Colors.END = ''


class RuleReviewer:
    """Interactive rule review tool"""

    def __init__(self, json_path: str):
        self.json_path = json_path
        self.data = None
        self.current_index = 0
        self.load_data()

    def load_data(self):
        """Load the JSON file"""
        try:
            with open(self.json_path, 'r') as f:
                self.data = json.load(f)
        except FileNotFoundError:
            print(f"{Colors.RED}Error: File not found: {self.json_path}{Colors.END}")
            sys.exit(1)
        except json.JSONDecodeError as e:
            print(f"{Colors.RED}Error: Invalid JSON: {e}{Colors.END}")
            sys.exit(1)

    def save_data(self):
        """Save the JSON file with pretty formatting"""
        try:
            with open(self.json_path, 'w') as f:
                json.dump(self.data, f, indent=2)
            return True
        except Exception as e:
            print(f"{Colors.RED}Error saving file: {e}{Colors.END}")
            return False

    def clear_screen(self):
        """Clear the terminal screen"""
        os.system('clear' if os.name == 'posix' else 'cls')

    def get_rules(self) -> List[Dict]:
        """Get the list of rules"""
        return self.data.get('rules', [])

    def get_rule(self, index: int) -> Optional[Dict]:
        """Get a specific rule by index"""
        rules = self.get_rules()
        if 0 <= index < len(rules):
            return rules[index]
        return None

    def display_rule(self, rule: Dict, index: int):
        """Display a rule in a human-friendly format"""
        self.clear_screen()

        total = len(self.get_rules())
        progress = f"Rule {index + 1} of {total}"

        # Header
        print(f"{Colors.BOLD}{Colors.HEADER}{'=' * 80}{Colors.END}")
        print(f"{Colors.BOLD}{Colors.HEADER}{progress:^80}{Colors.END}")
        print(f"{Colors.BOLD}{Colors.HEADER}{'=' * 80}{Colors.END}\n")

        # Rule ID and Name
        print(f"{Colors.BOLD}{Colors.CYAN}Rule ID:{Colors.END} {rule.get('id', 'N/A')}")
        print(f"{Colors.BOLD}{Colors.CYAN}Category:{Colors.END} {rule.get('category', 'N/A')}")
        print(f"{Colors.BOLD}{Colors.CYAN}Name:{Colors.END} {rule.get('name', 'N/A')}")
        print(f"{Colors.BOLD}{Colors.CYAN}Short Name:{Colors.END} {rule.get('shortName', 'N/A')}")
        print(f"{Colors.BOLD}{Colors.CYAN}Severity:{Colors.END} {rule.get('severity', 'N/A').upper()}\n")

        # Description
        print(f"{Colors.BOLD}{Colors.BLUE}Description:{Colors.END}")
        desc = rule.get('description', 'N/A')
        self.wrap_text(desc, 78)
        print()

        # Rationale
        print(f"{Colors.BOLD}{Colors.BLUE}Rationale:{Colors.END}")
        rationale = rule.get('rationale', 'N/A')
        self.wrap_text(rationale, 78)
        print()

        # DSL Pattern
        if 'dsl' in rule:
            print(f"{Colors.BOLD}{Colors.BLUE}DSL Pattern:{Colors.END}")
            print(f"  Pattern: {rule['dsl'].get('pattern', 'N/A')}")
            print(f"  Rule: {rule['dsl'].get('rule', 'N/A')}")
            print()

        # Examples
        if 'examples' in rule:
            examples = rule['examples']
            if 'valid' in examples and examples['valid']:
                print(f"{Colors.BOLD}{Colors.GREEN}Valid Example:{Colors.END}")
                for ex in examples['valid'][:1]:  # Show first example
                    print(f"  {ex[:100]}{'...' if len(ex) > 100 else ''}")
                print()

            if 'invalid' in examples and examples['invalid']:
                print(f"{Colors.BOLD}{Colors.RED}Invalid Example:{Colors.END}")
                for ex in examples['invalid'][:1]:  # Show first example
                    print(f"  {ex[:100]}{'...' if len(ex) > 100 else ''}")
                print()

        # References
        if 'references' in rule and rule['references']:
            print(f"{Colors.BOLD}{Colors.BLUE}References:{Colors.END}")
            for ref in rule['references']:
                print(f"  - {ref}")
            print()

        # Existing review comment
        if 'review_comment' in rule and rule['review_comment']:
            print(f"{Colors.BOLD}{Colors.YELLOW}⚠ EXISTING REVIEW COMMENT:{Colors.END}")
            self.wrap_text(rule['review_comment'], 76, prefix="  ")
            print()

        # Status indicator
        if 'review_comment' in rule and rule['review_comment']:
            print(f"{Colors.YELLOW}Status: NEEDS REVISION{Colors.END}\n")
        else:
            print(f"{Colors.GREEN}Status: No comments{Colors.END}\n")

        print(f"{Colors.BOLD}{Colors.HEADER}{'-' * 80}{Colors.END}")

    def wrap_text(self, text: str, width: int, prefix: str = "  "):
        """Wrap text to specified width"""
        words = text.split()
        lines = []
        current_line = prefix

        for word in words:
            if len(current_line) + len(word) + 1 <= width:
                if current_line == prefix:
                    current_line += word
                else:
                    current_line += " " + word
            else:
                lines.append(current_line)
                current_line = prefix + word

        if current_line:
            lines.append(current_line)

        for line in lines:
            print(line)

    def display_menu(self, rule: Dict):
        """Display the action menu"""
        print(f"\n{Colors.BOLD}Actions:{Colors.END}")

        has_comment = 'review_comment' in rule and rule['review_comment']

        if has_comment:
            print(f"  {Colors.YELLOW}[r]{Colors.END} Replace comment")
            print(f"  {Colors.GREEN}[c]{Colors.END} Clear comment (mark as OK)")
        else:
            print(f"  {Colors.YELLOW}[r]{Colors.END} Add revision comment")

        print(f"  {Colors.CYAN}[n]{Colors.END} Next rule")
        print(f"  {Colors.CYAN}[p]{Colors.END} Previous rule")
        print(f"  {Colors.CYAN}[s]{Colors.END} Skip to rule number")
        print(f"  {Colors.CYAN}[l]{Colors.END} List all rules")
        print(f"  {Colors.CYAN}[f]{Colors.END} Filter rules (with/without comments)")
        print(f"  {Colors.BLUE}[h]{Colors.END} Help")
        print(f"  {Colors.RED}[q]{Colors.END} Quit (saves automatically)")
        print()

    def add_comment(self, rule: Dict) -> bool:
        """Add or update a review comment for a rule"""
        print(f"\n{Colors.BOLD}Enter your review comment:{Colors.END}")
        print(f"{Colors.CYAN}(Press Enter on empty line to finish, or type 'cancel' to abort){Colors.END}\n")

        lines = []
        while True:
            try:
                line = input("> ")
                if line.lower() == 'cancel':
                    print(f"{Colors.YELLOW}Comment cancelled.{Colors.END}")
                    input("\nPress Enter to continue...")
                    return False
                if line == "" and len(lines) > 0:
                    break
                if line:
                    lines.append(line)
            except (EOFError, KeyboardInterrupt):
                print(f"\n{Colors.YELLOW}Comment cancelled.{Colors.END}")
                input("\nPress Enter to continue...")
                return False

        if lines:
            comment = " ".join(lines)
            rule['review_comment'] = comment
            if self.save_data():
                print(f"\n{Colors.GREEN}✓ Comment saved successfully!{Colors.END}")
            else:
                print(f"\n{Colors.RED}✗ Error saving comment.{Colors.END}")
            input("\nPress Enter to continue...")
            return True
        else:
            print(f"{Colors.YELLOW}No comment entered.{Colors.END}")
            input("\nPress Enter to continue...")
            return False

    def clear_comment(self, rule: Dict) -> bool:
        """Clear the review comment from a rule"""
        if 'review_comment' in rule:
            print(f"\n{Colors.YELLOW}Clear the review comment for this rule?{Colors.END}")
            confirm = input("Type 'yes' to confirm: ").strip().lower()
            if confirm == 'yes':
                del rule['review_comment']
                if self.save_data():
                    print(f"{Colors.GREEN}✓ Comment cleared successfully!{Colors.END}")
                else:
                    print(f"{Colors.RED}✗ Error saving changes.{Colors.END}")
                input("\nPress Enter to continue...")
                return True
        print(f"{Colors.YELLOW}No comment to clear.{Colors.END}")
        input("\nPress Enter to continue...")
        return False

    def list_all_rules(self):
        """Display a list of all rules with their status"""
        self.clear_screen()
        print(f"{Colors.BOLD}{Colors.HEADER}All Rules{Colors.END}\n")

        rules = self.get_rules()
        for i, rule in enumerate(rules, 1):
            has_comment = 'review_comment' in rule and rule['review_comment']
            status = f"{Colors.YELLOW}[NEEDS REVIEW]{Colors.END}" if has_comment else f"{Colors.GREEN}[OK]{Colors.END}"

            print(f"{i:2d}. {status} {Colors.BOLD}{rule.get('id', 'N/A')}{Colors.END} - {rule.get('shortName', 'N/A')}")

        print(f"\n{Colors.CYAN}Total rules: {len(rules)}{Colors.END}")

        # Count rules with comments
        commented = sum(1 for r in rules if 'review_comment' in r and r['review_comment'])
        if commented > 0:
            print(f"{Colors.YELLOW}Rules needing revision: {commented}{Colors.END}")

        input("\nPress Enter to continue...")

    def filter_rules(self):
        """Navigate to rules with or without comments"""
        print(f"\n{Colors.BOLD}Filter rules:{Colors.END}")
        print(f"  {Colors.YELLOW}[1]{Colors.END} Show only rules WITH comments (need revision)")
        print(f"  {Colors.GREEN}[2]{Colors.END} Show only rules WITHOUT comments (OK)")
        print(f"  {Colors.CYAN}[c]{Colors.END} Cancel")

        choice = input("\nChoice: ").strip().lower()

        rules = self.get_rules()
        filtered = []

        if choice == '1':
            filtered = [i for i, r in enumerate(rules) if 'review_comment' in r and r['review_comment']]
            if not filtered:
                print(f"\n{Colors.GREEN}No rules with comments found!{Colors.END}")
                input("Press Enter to continue...")
                return
            print(f"\n{Colors.YELLOW}Found {len(filtered)} rules with comments{Colors.END}")
        elif choice == '2':
            filtered = [i for i, r in enumerate(rules) if 'review_comment' not in r or not r['review_comment']]
            if not filtered:
                print(f"\n{Colors.GREEN}All rules have comments!{Colors.END}")
                input("Press Enter to continue...")
                return
            print(f"\n{Colors.GREEN}Found {len(filtered)} rules without comments{Colors.END}")
        else:
            return

        # Display filtered list
        for idx in filtered:
            rule = rules[idx]
            print(f"  {idx + 1}. {Colors.BOLD}{rule.get('id', 'N/A')}{Colors.END} - {rule.get('shortName', 'N/A')}")

        print(f"\n{Colors.CYAN}Enter rule number to jump to, or press Enter to cancel{Colors.END}")
        jump = input("Rule #: ").strip()

        if jump.isdigit():
            target = int(jump) - 1
            if target in filtered:
                self.current_index = target
            else:
                print(f"{Colors.RED}Invalid rule number{Colors.END}")
                input("Press Enter to continue...")

    def show_help(self):
        """Display help information"""
        self.clear_screen()
        print(f"{Colors.BOLD}{Colors.HEADER}HeliOS Rule Review Tool - Help{Colors.END}\n")

        print(f"{Colors.BOLD}Purpose:{Colors.END}")
        print("  This tool allows you to review each coding standard rule and mark rules")
        print("  that need revision by adding comments. Comments are saved to the JSON file")
        print("  and can be addressed later.\n")

        print(f"{Colors.BOLD}Navigation:{Colors.END}")
        print("  - Use 'n' and 'p' to move between rules")
        print("  - Use 's' to jump to a specific rule number")
        print("  - Use 'l' to see a list of all rules")
        print("  - Use 'f' to filter and navigate to rules with/without comments\n")

        print(f"{Colors.BOLD}Adding Comments:{Colors.END}")
        print("  - Press 'r' to add or replace a comment")
        print("  - Enter your comment (can be multiple lines)")
        print("  - Press Enter on an empty line to finish")
        print("  - Type 'cancel' to abort\n")

        print(f"{Colors.BOLD}Clearing Comments:{Colors.END}")
        print("  - Press 'c' to clear a comment (marks rule as OK)")
        print("  - Confirm by typing 'yes'\n")

        print(f"{Colors.BOLD}Status Indicators:{Colors.END}")
        print(f"  {Colors.YELLOW}[NEEDS REVIEW]{Colors.END} - Rule has a review comment")
        print(f"  {Colors.GREEN}[OK]{Colors.END} - Rule has no comments\n")

        print(f"{Colors.BOLD}Tips:{Colors.END}")
        print("  - The file is automatically saved after each comment change")
        print("  - You can run this tool multiple times to review and update")
        print("  - Use 'q' to quit at any time\n")

        input("Press Enter to continue...")

    def skip_to_rule(self):
        """Jump to a specific rule number"""
        total = len(self.get_rules())
        print(f"\n{Colors.CYAN}Enter rule number (1-{total}):{Colors.END}")
        try:
            num = input("Rule #: ").strip()
            if num.isdigit():
                index = int(num) - 1
                if 0 <= index < total:
                    self.current_index = index
                    return
            print(f"{Colors.RED}Invalid rule number{Colors.END}")
            input("Press Enter to continue...")
        except (EOFError, KeyboardInterrupt):
            pass

    def run(self):
        """Main review loop"""
        rules = self.get_rules()
        total = len(rules)

        if total == 0:
            print(f"{Colors.RED}No rules found in the JSON file.{Colors.END}")
            return

        print(f"{Colors.BOLD}{Colors.GREEN}HeliOS Coding Standard Rule Review Tool{Colors.END}")
        print(f"\nReady to review {total} rules.")
        print(f"Type 'h' for help at any time.\n")
        input("Press Enter to start...")

        while True:
            rule = self.get_rule(self.current_index)
            if not rule:
                break

            self.display_rule(rule, self.current_index)
            self.display_menu(rule)

            try:
                choice = input(f"{Colors.BOLD}Choose action: {Colors.END}").strip().lower()
            except (EOFError, KeyboardInterrupt):
                print(f"\n{Colors.YELLOW}Interrupted. Quitting...{Colors.END}")
                break

            if choice == 'q':
                print(f"\n{Colors.GREEN}Goodbye!{Colors.END}")
                break
            elif choice == 'r':
                self.add_comment(rule)
            elif choice == 'c':
                self.clear_comment(rule)
            elif choice == 'n':
                if self.current_index < total - 1:
                    self.current_index += 1
                else:
                    print(f"{Colors.YELLOW}Already at last rule.{Colors.END}")
                    input("Press Enter to continue...")
            elif choice == 'p':
                if self.current_index > 0:
                    self.current_index -= 1
                else:
                    print(f"{Colors.YELLOW}Already at first rule.{Colors.END}")
                    input("Press Enter to continue...")
            elif choice == 's':
                self.skip_to_rule()
            elif choice == 'l':
                self.list_all_rules()
            elif choice == 'f':
                self.filter_rules()
            elif choice == 'h':
                self.show_help()
            else:
                print(f"{Colors.YELLOW}Invalid choice. Press 'h' for help.{Colors.END}")
                input("Press Enter to continue...")


def main():
    """Main entry point"""
    # Check if we're in a terminal that supports colors
    if not sys.stdout.isatty():
        Colors.disable()

    # Get the JSON file path
    script_dir = os.path.dirname(os.path.abspath(__file__))
    json_file = os.path.join(script_dir, "HELIOS_CODE_STANDARD.json")

    if not os.path.exists(json_file):
        print(f"{Colors.RED}Error: HELIOS_CODE_STANDARD.json not found in {script_dir}{Colors.END}")
        sys.exit(1)

    reviewer = RuleReviewer(json_file)
    reviewer.run()


if __name__ == "__main__":
    main()
