//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

class RubiksCube {

public:
  enum move_type_t { FC, FCC, DC, DCC, LC, LCC, NUMMOVES };

private:
  static const size_t NUMSLOTS = 24;

  typedef uint16_t facelet_id_t;
  facelet_id_t mSlots[NUMSLOTS];

  // X - Axis: { Front = 0, Back = 1 }
  // Y - Axis: { Left = 0, Right = 1 }
  // Z - Axis: { Down = 0, Up = 1 }
  // Face    : {X-facing = 0, Y-facing = 1, Z-facing = 2}

  static size_t get_slot_num(uint8_t x, uint8_t y, uint8_t z, uint8_t f) {
    return (x << 2 | y << 1 | z) * 3 + f;
  }

  static std::string get_slot_str(size_t slot) {
    const auto s = slot / 3;
    std::string str[] = {(((s >> 2) & 0x1) ? "B" : "F"),
                         (((s >> 1) & 0x1) ? "R" : "L"),
                         ((s & 0x1) ? "U" : "D")};

    str[slot % 3] = std::string("(") + str[slot % 3] + ")";
    return str[0] + str[1] + str[2];
  }

  enum color_t { R, G, B, C, M, Y }; // 6 colors for 6 faces.
  static facelet_id_t get_facelet_id(color_t c1, color_t c2, color_t c3) {
    return (c1 << 6 | c2 << 3 | c3);
  }

  // Extract the primary color from the facelet-id.
  static color_t get_facelet_color(facelet_id_t f) {
    return static_cast<color_t>((f >> 6) & 0x7);
  }

  static std::string get_cubelet_str(facelet_id_t cid) {
    auto get_color_str = [](int c) -> std::string {
      switch (c) {
      case R:
        return "R";
      case G:
        return "G";
      case B:
        return "B";
      case C:
        return "C";
      case M:
        return "M";
      case Y:
        return "Y";
      default:
        break;
      }
      return "";
    };

    return get_color_str((cid >> 6) & 0x7) + get_color_str((cid >> 3) & 0x7) +
           get_color_str(cid & 0x7);
  }

  static size_t Move[NUMMOVES][NUMSLOTS];

  static void init_moves() {
    if (Move[0][0] != 0) // Already initialized.
      return;

    for (auto i = 0; i < NUMMOVES; ++i)
      for (auto j = 0U; j < NUMSLOTS; ++j)
        Move[i][j] = j;

    auto populate_move = [](move_type_t mv, size_t cubelet_from[4][3],
                            size_t cubelet_to[4][3], size_t facelet_from[3],
                            size_t facelet_to[3]) {
      for (auto cubelet = 0; cubelet < 4; ++cubelet)
        for (auto facelet = 0; facelet < 3; ++facelet)
          RubiksCube::Move[mv][get_slot_num(
              cubelet_from[cubelet][0], cubelet_from[cubelet][1],
              cubelet_from[cubelet][2], facelet_from[facelet])] =
              get_slot_num(cubelet_to[cubelet][0], cubelet_to[cubelet][1],
                           cubelet_to[cubelet][2], facelet_to[facelet]);
    };
    //////////////////////////////////////////////////////
    // Front - Clockwise Move
    // Cubelet (F, L, D) [0, 0, 0] ==> (F, L, U) [0, 0, 1]
    // Cubelet (F, L, U) [0, 0, 1] ==> (F, R, U) [0, 1, 1]
    // Cubelet (F, R, U) [0, 1, 1] ==> (F, R, D) [0, 1, 0]
    // Cubelet (F, R, D) [0, 1, 0] ==> (F, L, D) [0, 0, 0]
    //////////////////////////////////////////////////////
    {
      size_t cbltf[4][3] = {{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}};
      size_t cbltt[4][3] = {{0, 0, 1}, {0, 1, 1}, {0, 1, 0}, {0, 0, 0}};

      size_t faceletf[] = {0, 1, 2};
      size_t facelett[] = {0, 2, 1};

      populate_move(FC, cbltf, cbltt, faceletf, facelett);
    }
    //////////////////////////////////////////////////////
    // Left - Clockwise Move
    // Cubelet (F, L, D) [0, 0, 0] ==> (F, L, U) [0, 0, 1]
    // Cubelet (F, L, U) [0, 0, 1] ==> (B, L, U) [1, 0, 1]
    // Cubelet (B, L, U) [1, 0, 1] ==> (B, L, D) [1, 0, 0]
    // Cubelet (B, L, D) [1, 0, 0] ==> (F, L, D) [0, 0, 0]
    //////////////////////////////////////////////////////
    {
      size_t cbltf[4][3] = {{0, 0, 0}, {0, 0, 1}, {1, 0, 1}, {1, 0, 0}};
      size_t cbltt[4][3] = {{0, 0, 1}, {1, 0, 1}, {1, 0, 0}, {0, 0, 0}};

      size_t faceletf[] = {0, 1, 2};
      size_t facelett[] = {2, 1, 0};

      populate_move(LC, cbltf, cbltt, faceletf, facelett);
    }
    //////////////////////////////////////////////////////
    // Down - Clockwise Move
    // Cubelet (F, L, D) [0, 0, 0] ==> (F, R, D) [0, 1, 0]
    // Cubelet (F, R, D) [0, 1, 0] ==> (B, R, D) [1, 1, 0]
    // Cubelet (B, R, D) [1, 1, 0] ==> (B, L, D) [1, 0, 0]
    // Cubelet (B, L, D) [1, 0, 0] ==> (F, L, D) [0, 0, 0]
    //////////////////////////////////////////////////////
    {
      size_t cbltf[4][3] = {{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}};
      size_t cbltt[4][3] = {{0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}};

      size_t faceletf[] = {0, 1, 2};
      size_t facelett[] = {1, 0, 2};

      populate_move(DC, cbltf, cbltt, faceletf, facelett);
    }

    // Counter clockwise moves are inverse of clockwise moves.
    move_type_t mc[3] = {FC, LC, DC};
    move_type_t mcc[3] = {FCC, LCC, DCC};
    for (auto m = 0; m < 3; ++m) {
      for (auto i = 0U; i < NUMSLOTS; ++i) {
        auto v = Move[mc[m]][i];
        Move[mcc[m]][v] = i;
      }
    }
  }

public:
  RubiksCube() {
    init_moves();

    const color_t face_color[][2] = {
        {R, G}, // X-Face colors
        {B, C}, // Y-Face colors
        {M, Y}  // Z-Face colors
    };

    for (uint8_t x = 0; x < 2; ++x)
      for (uint8_t y = 0; y < 2; ++y)
        for (uint8_t z = 0; z < 2; ++z)
          for (uint8_t f = 0; f < 3; ++f) {
            color_t cubelet_color[] = {face_color[0][x], face_color[1][y],
                                       face_color[2][z]};
            auto facelet_id =
                get_facelet_id(cubelet_color[f], cubelet_color[(f + 1) % 3],
                               cubelet_color[(f + 2) % 3]);
            auto slot = get_slot_num(x, y, z, f);
            mSlots[slot] = facelet_id;
          }
  }

  void apply_move(move_type_t m) {
    if (m >= NUMMOVES || m < 0)
      return;

    size_t oldSlots[NUMSLOTS];
    std::copy(mSlots, &mSlots[NUMSLOTS], oldSlots);
    for (auto i = 0U; i < NUMSLOTS; ++i)
      mSlots[i] = oldSlots[Move[m][i]];
  }

  bool is_solved() const {
    // Check all faces have the same color.
    for (uint8_t x = 0; x < 2; ++x)
      for (uint8_t y = 0; y < 2; ++y)
        for (uint8_t z = 0; z < 2; ++z)
          for (uint8_t f = 0; f < 3; ++f) {
            auto fcolor = get_facelet_color(mSlots[get_slot_num(
                (f == 0) ? x : 0, (f == 1) ? y : 0, (f == 2) ? z : 0, f)]);
            auto slot = get_slot_num(x, y, z, f);
            if (get_facelet_color(mSlots[slot]) != fcolor) {
              return false;
            }
          }

    return true;
  }

  friend std::ostream &operator<<(std::ostream &os, const RubiksCube &rc);
};

size_t RubiksCube::Move[NUMMOVES][NUMSLOTS] = {0};

std::ostream &operator<<(std::ostream &os, const RubiksCube &rc) {
  for (size_t slot = 0; slot < RubiksCube::NUMSLOTS; ++slot) {
    os << "[" << RubiksCube::get_slot_str(slot)
       << "] = " << RubiksCube::get_cubelet_str(rc.mSlots[slot]) << std::endl;
  }
  os << ((rc.is_solved()) ? "SOLVED" : "UNSOLVED") << std::endl;
  return os;
}

// Let a monkey play with the cube.
size_t monkey_play(RubiksCube &r) {
  const size_t nmoves = random() % 200;
  for (auto i = 0U; i < nmoves; ++i) {
    const RubiksCube::move_type_t move =
        static_cast<RubiksCube::move_type_t>(random() % RubiksCube::NUMMOVES);
    r.apply_move(move);
  }

  return nmoves;
}

int main() {
  RubiksCube r;
  std::cout << r << std::endl;
  monkey_play(r);
  std::cout << r << std::endl;
  return 0;
}
