// -- this doesn't work :(
// -- and there's some other variant with throw which works but the error msg is garbage
// template <uint8_t max>
// struct Fin {
//   uint8_t n;
//   consteval Fin(uint8_t n_raw) : n(n_raw) {
//     if (n_raw >= max) { static_assert(false, "Value is outside of the Fin range"); }
//   }
// };

