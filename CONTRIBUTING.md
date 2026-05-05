# Contributing to ORION Common

Thank you for your interest in contributing to the ORION project! This document provides guidelines to ensure code quality and consistency.

---

## Code Standards

### C++ Code

All C++ code must follow ROS 2 best practices and pass `ament_clang_format`:

```bash
cd ~/ros2_ws
ament_clang_format src/orion_common --reformat
```

**Style guidelines:**
- Use snake_case for variables and function names
- Use PascalCase for class names
- Maximum line length: 100 characters
- Prefer `auto` for iterator types
- Use `const` and `constexpr` liberally

### Python Code

All Python code must follow ROS 2 conventions and pass `flake8` and `pep257`:

```bash
cd ~/ros2_ws
ament_flake8 src/orion_common
ament_pep257 src/orion_common
```

**Style guidelines:**
- PEP 8 compliant
- Type hints preferred for public functions
- Docstrings for all public functions (Google style)

---

## Creating a Pull Request

### Branch Naming Convention

Use the format: `author/action/package/number`

**Examples:**
- `danflopez/feature/orion_control/123`
- `danflopez/fix/orion_bringup/45`
- `danflopez/docs/orion_utils_py/67`
- `miguelgonrod/refactor/orion_description/89`

Where:
- `author`: Your GitHub username
- `action`: `feature`, `fix`, `docs`, `refactor`, etc.
- `package`: The main package affected (e.g., `orion_control`, `orion_base`)
- `number`: Issue or ticket number (optional but recommended)

### Commit Messages

Be descriptive and concise:
- Format: `package: short summary`
- Example: `orion_control: fix PID clamping in DiffDriveOrion`

### PR Description

Include:
- What changed and why
- Related issues (if any)
- Testing performed

### Review Process

All PRs require at least one review before merging.

---

## Documentation

- Update READMEs if your change affects user-facing behavior
- Keep comments minimal; use clear naming instead
- Only comment the "why," not the "what"

---

## Versioning

This project follows semantic versioning (MAJOR.MINOR.PATCH).
Version bumps are decided by maintainers during releases.

---

## Questions?

Open an issue or reach out to the maintainers.
