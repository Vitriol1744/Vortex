# Project templates

Each directory here is a template Nexus (and Horizon's New project dialog)
offers when creating a project. They ship with the engine, so every engine
version creates projects its own way.

```
<id>/template.json     what the template is (required)
<id>/content/...       copied into the new project as is (optional)
```

`template.json`:

| Key           | Meaning                                                        |
|---------------|----------------------------------------------------------------|
| `name`        | Shown in the template list                                     |
| `description` | One or two sentences under the name                            |
| `category`    | A short label, e.g. `3D`, `2D`, `Any`                          |
| `icon`        | `file`, `cube`, `bolt`, `rocket`, `gamepad`, `globe`...        |
| `order`       | Lower comes first                                              |
| `startScene`  | The project's start scene, relative to the project root       |
| `generator`   | Optional: Nexus builds the start scene in code (`physics`, `empty`) |

`content/` should hold the start scene unless a generator writes it; if
neither does, the project starts with `assets/scenes/cube.vscene`.

Pick one from the command line with
`Nexus --new-project <dir> --name <name> --template <id>`; the default is
`starter-3d`.
