#include <iostream>
#include <chrono>

#define PRINT_INTERMEDIATE_STEPS 0

class DiagonalGame {
public:
    enum Diagonal { EMPTY, FORWARD, BACKWARD};

    explicit DiagonalGame(size_t grid_size=5, unsigned int max_diags=16);
    bool play(unsigned int i, unsigned int j, Diagonal type);
    ~DiagonalGame();
    void printSolution() const;
private:
    Diagonal **matrix;
    size_t n; // number of rows and columns in the grid
    size_t n_from_0; // number of rows and columns in the grid - 1
    unsigned int max_diags;
    unsigned int n_diags = 0;

    bool extend(int i, int j);
    bool putDiagonal(int i, int j, Diagonal type, bool just_check);
    bool removeDiagonal(int i, int j);
};

DiagonalGame::DiagonalGame(size_t grid_size, unsigned int max_diags) {
    if (max_diags >= grid_size * grid_size) {
        std::cerr << "Invalid number of diagonals" << std::endl;
        return;
    }

    this->n = grid_size;
    this->n_from_0 = grid_size - 1;
    this->max_diags = max_diags;

    this->matrix = new Diagonal*[grid_size];
    // build the matrix
    size_t j = 0;
    for (size_t i = 0; i < grid_size; ++i) {
        this->matrix[i] = new Diagonal[grid_size];

        for (j = 0; j < grid_size; ++j) // fill the matrix with empty values
            this->matrix[i][j] = this->EMPTY;
    }
}

bool DiagonalGame::putDiagonal(const int i, const int j, const Diagonal type, bool just_check=false) {
    bool has_more_cols = j < this->n_from_0, has_more_rows = i < this->n_from_0;
    bool has_less_cols = j > 0, has_less_rows = i > 0;

    if (i < 0 || j < 0 || i >= this->n || j >= this->n)
        return false;
    if (this->matrix[i][j] != this->EMPTY)
        return false;

    if (type == this->FORWARD) {
        // check row above
        if (has_less_rows) {
            if (this->matrix[i - 1][j] == this->BACKWARD) // top
                return false;
            if (has_more_cols && this->matrix[i - 1][j + 1] == this->FORWARD) // top-right
                return false;
        }
        // check cols to the side
        if (has_more_cols && this->matrix[i][j + 1] == this->BACKWARD) // right
            return false;
        if (has_less_cols && this->matrix[i][j - 1] == this->BACKWARD) // left
            return false;
        // check row below
        if (has_more_rows) {
            if (this->matrix[i + 1][j] == this->BACKWARD) // bottom
                return false;
            if (has_less_cols && this->matrix[i + 1][j - 1] == this->FORWARD) // bottom-left
                return false;
        }
        if (!just_check) {
            this->matrix[i][j] = this->FORWARD;
            ++(this->n_diags);
        }
    } else if (type == this->BACKWARD) {
        // check row above
        if (has_less_rows) {
            if (this->matrix[i - 1][j] == this->FORWARD)
                return false;
            if (has_less_cols && this->matrix[i - 1][j - 1] == this->BACKWARD)
                return false;
        }
        // check cols to the side
        if (has_less_cols && this->matrix[i][j - 1] == this->FORWARD)
            return false;
        if (has_more_cols && this->matrix[i][j + 1] == this->FORWARD)
            return false;
        // check row below
        if (has_more_rows) {
            if (this->matrix[i + 1][j] == this->FORWARD)
                return false;
            if (has_more_cols && this->matrix[i + 1][j + 1] == this->BACKWARD)
                return false;
        }
        if (!just_check) {
            this->matrix[i][j] = this->BACKWARD;
            ++(this->n_diags);
        }
    }
    return true;
}

bool DiagonalGame::removeDiagonal(int i, int j) {
    if (i < 0 || j < 0 || i >= this->n || j >= this->n)
        return false;

    this->matrix[i][j] = this->EMPTY;
    --this->n_diags;
    return true;
}

/**
 * This is the code to solve the game
 * First put some diagonal near to another diagonal and check if that can be extended and if it's a solution
 * if it's not, try to to the same on the other side of the board without erasing the first extension
 * if that didn't work, erase both extensions
 * Do this recursively until you find a solution or every function call return false (which means no solution was found)
 *
 * A cell, row-col is expended when all cells immediately near it are being extended
 * this means that for each one of these cells we try to put a diagonal to see if it's a solution
 *
 * TODO: all the ones describe below, but also, try the inputs 6 (grid size), 20 (diagonals), (or 7 and 21, or 5 and 17, there are many options) the output will be 21 diagonals found, it's good but it'd be nice if the output is exactly 20
 * although I think that to achieve that an extra variable is needed to know and compare after each expansion if we've reached the desired # of diagonals
 * @param i, int the row of the diagonal to extend
 * @param j, int the col of the diagonal to extend
 * @return false if no solution has found (if the expansion is not a solution), true if the number of diagonals have been reached
 */
bool DiagonalGame::extend(int i, int j) {
    if (i < 0 || j < 0 || i >= this->n || j >= this->n)
        return false;
#if PRINT_INTERMEDIATE_STEPS
    this->printSolution();
#endif
    // try with both types backslash and forward slash
    Diagonal diag_type;
    int i_other_side, j_other_side;
    for (short diag_type_i = 0; diag_type_i < 2; ++diag_type_i) {
        diag_type = diag_type_i == 0 ? this->FORWARD : this->BACKWARD;

        for (int k = j - 1; k <= j + 1; ++k) { // try with the above and below rows
            if (k < 0) // we don't wanna use negative indexes
                continue;
            // TODO: more checks should been done here, to avoid calling putDiagonal many times
            // TODO: that function validates the indexes, but still, more checks should take place here

            // try the row above this one, [i - 1, j - 1], [i - 1, j], [i - 1, j + 1]
            if (i > 0) {
                if (this->putDiagonal(i - 1, k, diag_type)) {
                    if (!this->extend(i - 1, k)) { // if could not extend on this side
                        // try from the other side of the board
                        i_other_side = this->n_from_0 - i + 1;
                        j_other_side = this->n_from_0 - k;
                        if (this->putDiagonal(i_other_side, j_other_side, diag_type)) {
                            if (!this->extend(i_other_side, j_other_side)) {
                                this->removeDiagonal(i_other_side, j_other_side); // undo changes
                                this->removeDiagonal(i - 1, k); // undo changes
                            }
                        } else
                            this->removeDiagonal(i - 1, k); // undo changes
                    }
                }
            }

            // try the row below this one, [i + 1, j - 1], [i + 1, j], [i + 1, j + 1]
            if (i < this->n) {
                if (this->putDiagonal(i + 1, k, diag_type)) {
                    if (!this->extend(i + 1, k)) {
                        // try from the other side of the board
                        i_other_side = this->n_from_0 - i - 1;
                        j_other_side = this->n_from_0 - k;
                        if (this->putDiagonal(i_other_side, j_other_side, diag_type)) {
                            if (!this->extend(i_other_side, j_other_side)) {
                                this->removeDiagonal(i_other_side, j_other_side); // undo changes
                                this->removeDiagonal(i + 1, k); // undo changes
                            }
                        } else
                            this->removeDiagonal(i + 1, k); // undo changes
                    }
                }
            }
        }

        // try the left side, [i, j - 1]
        if (j > 0) {
            if (this->putDiagonal(i, j - 1, diag_type) && !this->extend(i, j - 1)) {
                // try from the other side of the board
                i_other_side = this->n_from_0 - i;
                j_other_side = this->n_from_0 - j + 1;
                if (this->putDiagonal(i_other_side, j_other_side, diag_type)) {
                    if (!this->extend(i_other_side, j_other_side)) {
                        this->removeDiagonal(i_other_side, j_other_side);
                        this->removeDiagonal(i, j - 1); // undo changes
                    }
                } else
                    this->removeDiagonal(i, j - 1); // undo changes
            }
        }

        // try the right side, [i, j + 1]
        if (j < this->n) {
            if (this->putDiagonal(i, j + 1, diag_type) && !this->extend(i, j + 1)) {
                // try from the other side of the board
                i_other_side = this->n_from_0 - i;
                j_other_side = this->n_from_0 - j - 1;
                if (this->putDiagonal(i_other_side, j_other_side, diag_type)) {
                    if (!this->extend(i_other_side, j_other_side)) {
                        this->removeDiagonal(i_other_side, j_other_side);
                        this->removeDiagonal(i, j + 1); // undo changes
                    }
                } else
                    this->removeDiagonal(i, j + 1); // undo changes
            }
        }
    }

    return this->n_diags >= this->max_diags;
}

void DiagonalGame::printSolution() const {
    for (unsigned int i = 0; i < this->n; ++i) {
        for (unsigned int j = 0; j < this->n; ++j) {
            std::cout << ' ';
            if (matrix[i][j] == this->FORWARD)
                std::cout << '/';
            else if (matrix[i][j] == this->BACKWARD)
                std::cout << '\\';
            else
                std::cout << '-';
            std::cout << " |";
        }
        std::cout << std::endl;
    }
    std::cout << "# diagonals: " << this->n_diags << std::endl;
}

bool DiagonalGame::play(unsigned int i = 0, unsigned int j = 0, Diagonal type = FORWARD) {
    if (type != this->FORWARD && type != this->BACKWARD) {
        std::cerr << "Invalid diagonal" << std::endl; // throw exception instead
        return false;
    }
    if (!(i <= this->n && j <= this->n)) {
        std::cerr << "Invalid initial position" << std::endl; // also throw exception instead
        return false;
    }

    this->matrix[i][j] = type;
    ++(this->n_diags);

    auto time_start = std::chrono::high_resolution_clock::now();
    bool result = this->extend(i, j);
    auto time_end = std::chrono::high_resolution_clock::now();

    std::cout << "The algorithm took: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count() << " ms" << std::endl;

    return result;
}

DiagonalGame::~DiagonalGame() {
    for (unsigned int i = 0; i < this->n; ++i) {
        delete[] this->matrix[i];
    }

    delete[] this->matrix;
}

int main() {
    // TODO: validate input
    unsigned int grid_size;
    std::cout << "Input grid size: ";
    std::cin >> grid_size;

    unsigned int max_diagonals;
    std::cout << "Input the # of diagonals to fill within the grid: ";
    std::cin >> max_diagonals;

    DiagonalGame game(grid_size, max_diagonals);

    unsigned int i, j;
    std::cout << "Set the initial diagonal" << std::endl;
    std::cout << "Initial ZERO-INDEXED coordinates of the diagonal (in the same line separated by spaces, e. .g 0 0): ";
    std::cin >> i >> j;

    unsigned int type = 0;
    std::cout << "Type of diagonal" << std::endl;
    std::cout << "1. Forward (/)\n2. Backward (\\)" << std::endl;
    std::cout << "Decision (1, 2): ";
    std::cin >> type;

    std::cout << (game.play(i, j, type == 1 ? game.FORWARD : game.BACKWARD) ? "There's a solution!!" : "No solution") << std::endl;
    game.printSolution();
}
