
# Theory: Difference Between Git Clone, Git Fetch, and Git Pull

# Git Clone
What it does: This command creates an exact, full copy of a remote GitHub repository on your local computer. It downloads all the project files, the entire commit history, and all existing branches. It also automatically sets up a tracking link called origin that points back to the source repository.

When to use it: You use git clone as your very first step when starting work on an existing project. For example, when you join a new project or want to work on a partner's repository, you clone it once to get a local working environment set up on your machine.

# Git Fetch
What it does: This command acts as a safe "check-in" with the remote repository. It reaches out to GitHub and downloads any new history, commits, or branches that your teammates have pushed, but it does not alter your current working files or merge anything into your active branch. It simply updates your local Git database tracking files.

When to use it: You use git fetch when you want to see what changes have been made on the remote repository without risking breaking your current local code. It allows you to safely inspect the remote status and compare histories before deciding to integrate the new updates.

# Git Pull
What it does: This command is an automated combination of two steps: it first runs git fetch to download the latest updates from the remote repository, and then it immediately runs git merge to fuse those remote changes directly into your current local active branch.

When to use it: You use git pull frequently throughout the day when you are ready to update your local files with the latest progress from GitHub. It keeps your local repository synchronized with your team, though you should ensure your local changes are committed first to avoid sudden merge conflicts.
