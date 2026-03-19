# Candy Crush PF Project

**Name:** Muhammad Umar Khan  
**Roll Number:** 25L-3089  
**Section:** BSE-1A  

## 1. Overview

This project is a simplified Candy Crush–style match-3 game built in C++ using SFML.  
You play on an 8×8 grid, swapping candies to make matches, create special candies, and reach a target score within a limited number of moves.

## 2. How to Run

1. Go to the **Executable** folder.
2. Make sure the following are in the same folder:
   - Game executable (e.g., `CandyCrush.exe`)
   - SFML DLLs
   - Resources:
     - **Images:** `menu.png`, `background.png`, `candy0.png`–`candy5.png`, `special4_candy*.png`, `special5_candy*.png`
     - **Sounds:** all `.mp3` files used in the code
     - **Font:** `arial.ttf`
3. Double-click the executable to run the game.

If running from an IDE, ensure SFML is set up correctly and the working directory points to the folder containing the resources.

## 3. How to Play

### Basic Rules

- The board is an 8×8 grid of candies.
- Use the mouse to click two adjacent candies (up, down, left, right) to swap them.
- A swap is valid only if it creates a match of 3 or more same-colored candies in a row or column; otherwise, the swap is reverted.

### Matches and Special Candies

- **3 in a row/column:**
  - Candies are removed.
  - Score increases by **10** for that match.

- **4 in a row/column:**
  - Score increases by **20** for that match.
  - One candy becomes a **3×3 bomb (type 1)**, which clears a 3×3 area when matched.

- **5 or more in a row/column:**
  - Score increases by **30** for that match.
  - One candy becomes a **row+column bomb (type 2)**, which clears its entire row and column when matched.

### Cascades and Scoring

- After a match:
  - Candies above fall down to fill gaps.
  - New candies spawn at the top.
  - New automatic matches (cascades) can occur.

- Only the first crush caused directly by the player’s swap gives score.  
  Cascades that follow do not increase the score; they only clear the board.

## 4. Game States

- **MENU:**
  - Start game, load game, open help screen.

- **PLAYING:**
  - Main gameplay with board, score, moves, and target.

- **GAMEOVER:**
  - **"You Win"** if target score is reached within moves.
  - **"You Lose"** if moves reach 0 before target.

- **HELP:**
  - Shows instructions and controls.

## 5. Controls

### From MENU

- **Enter:** Start a new game.
- **L:** Load saved game.
- **H:** Open Help screen.
- **Esc:** Quit application (or return to Menu from other states).

### From HELP

- **H** or **Esc:** Return to Menu.

### During GAME

- **Mouse Left Click:** Select and swap adjacent candies.
- **S:** Save current game and return to Menu.
- **M:** Toggle music ON/OFF.
- **F:** Toggle sound effects ON/OFF.
- **Esc:** Return to Menu (and switch to menu music).

### From GAMEOVER

- **Enter:** Start a new game.
- **Esc:** Return to Menu.

## 6. Save/Load

### Save

- Press **S** during gameplay.
- The game writes `score`, `movesLeft`, `grid`, and `special` arrays to `save.txt`.

### Load

- From Menu, press **L** to resume the saved game.
- Board, score, moves, and special candies are restored.

## 7. Features Implemented

- 8×8 match-3 grid with random initialization and no initial matches.
- Valid/invalid swap handling (invalid swaps revert).
- Match detection (horizontal and vertical).
- Special candies:
  - 4-match → 3×3 bomb (**type 1**)
  - 5-match → row+column bomb (**type 2**)
- Special candy effects:
  - 3×3 bomb clears a 3×3 area.
  - Row+column bomb clears entire row and column.
- Cascades (automatic matches after drops).
- Scoring system:
  - 3: **+10**
  - 4: **+20**
  - 5+: **+30** per first crush of the move
  - Cascades do not add extra score
- Move counter and target score with win/lose detection.
- Menu screen and separate Help screen.
- Save/Load system with `save.txt`.
- Background music (menu, gameplay, game over) with mute toggle.
- Sound effects for crush, bomb creation, and special explosions.
- Simple falling animation using `visualY`.

## 8. Known Limitations / Future Work

- Cascades resolve instantly; intermediate animation per cascade step is not shown, and no score is given for cascades as well.
- Bonus scoring for special candies (beyond base match points) was considered but removed to keep the logic simple and stable.
- No in-game pause menu besides returning to the main menu.
- No shuffling of board in case there are no valid moves on the board.
- No move timer.
- No move hints.
- No high score track.
