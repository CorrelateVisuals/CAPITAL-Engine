# Config parameters CAPITAL Engine

```C++
class Control {
 public:

  struct Timer {
    float speed = 100.0f;
  } timer;

  struct Grid {
    uint_fast32_t totalAliveCells = 5000;
    std::array<uint_fast16_t, 2> dimensions = {200, 150};
    float gap = 0.1f;
    float height = 0.1f;
  } grid;

  struct Cell {
    float size = {0.03f};
  } cells;
```

