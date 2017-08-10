Contributions to this project are welcome.

Copyright
---------
This project is meant as an educational source without any strings attached to the learner (read [LICENSE](LICENSE)). All contributions are made under the same lincense.

Issues
------
Catching bugs is important and so your Issue reports are very welcome.

- Issues about specific bugs in the source code should contain specification rule quote it violates.
- Other kinds of issues should contain info about OS, driver version, used Vulkan SDK and the output from validation layers.
- Relevant questions are also welcome as long as the trafic is low.
- New feature suggestions are pointless, unless you are willing to implement them yourself

Project goals and conventions
------------------------
This project aims to be a starting point in learning Vulkan API. It aims at people that are bad with tutorials. It tries not to insult the readers inteligence; it tries to do things the right/hard way and it tries not to hide things from the reader. Vulkan is a learning staircase; this is above all meant to be the first stair.

- Assume the reader has only vague idea about how Vulkan works. Show the actual commands; do not abstract too much. It should be readable even outsite an IDE.
  - There is a flat 2 level design. The top level shows the stream of actions using Vulkan terminology (strongly hinting at what Vulkan commands will be used). The second level contains the verbose Vulkan stuff (setting `CreateInfo`s and such).
  - Assume reader has some computer graphics knowledge; show specific Vulkan features, not effects or algorithms.
- The code must be valid and reasonably optimal Vulkan code. This requires carefully reading the [Vulkan specification](https://www.khronos.org/registry/vulkan/specs/1.0-extensions/html/vkspec.html) itself.
  - Suboptimal constructs (like `vkDeviceWaitIdle`s, or suboptimally set barriers) should be avoided.
- The code must not be platform dependent (unless it shows a specific non-universally supported feature).
- For now examples showing additional Vulkan features are kept as a diff (git branch) to the `master` (which represents the simplest viable Vulkan app). This is to allow easy comparison.

Pull requests
-------------
Pull requests following the conventions above are welcome.

Building should be straightforward and is outlined in [README.md](README.md).

If you need a new Branch just PR against `master`. It should be changeable later when new branch is created.
