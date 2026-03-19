package utils

// Point represents a 2D coordinate.
type Point struct {
	R, C int
}

// Dir4 contains the four cardinal directions (up, right, down, left).
var Dir4 = []Point{{-1, 0}, {0, 1}, {1, 0}, {0, -1}}

// Dir8 contains all eight directions including diagonals.
var Dir8 = []Point{{-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}}

// InBounds checks if a point is within grid dimensions.
func InBounds(p Point, rows, cols int) bool {
	return p.R >= 0 && p.R < rows && p.C >= 0 && p.C < cols
}

// ParseGrid converts lines into a 2D byte grid.
func ParseGrid(lines []string) [][]byte {
	grid := make([][]byte, len(lines))
	for i, line := range lines {
		grid[i] = []byte(line)
	}
	return grid
}

// FindInGrid returns the position of the first occurrence of a byte in the grid.
func FindInGrid(grid [][]byte, target byte) (Point, bool) {
	for r, row := range grid {
		for c, cell := range row {
			if cell == target {
				return Point{r, c}, true
			}
		}
	}
	return Point{}, false
}
