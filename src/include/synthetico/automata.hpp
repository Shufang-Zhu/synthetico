//
// Synthetico - Pure-past LTL synthesizer based on BLACK
//
// (C) 2023 Nicola Gigante
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef SYNTH_AUTOMATA_HPP
#define SYNTH_AUTOMATA_HPP

#include <black/logic/logic.hpp>

#include <synthetico/synth.hpp>

namespace synth {
  
  namespace logic = black::logic;

  struct automata {
    using formula = logic::formula<logic::propositional>;

    std::vector<logic::proposition> inputs;
    std::vector<logic::proposition> outputs;
    std::vector<logic::proposition> variables;

    formula init;
    formula trans;
    formula objective;
  };

  std::ostream &operator<<(std::ostream &, automata);

  struct primed_t {
    black::identifier label;

    bool operator==(primed_t const&) const = default;
  };

  inline std::string to_string(primed_t p) {
    return "{" + to_string(p.label) + "}'";
  }

  inline black::proposition primed(black::proposition p) {
    return p.sigma()->proposition(primed_t{p.name()});
  }

  automata encode(spec sp);

}

namespace std {
  template<>
  struct hash<::synth::primed_t> {
    size_t operator()(::synth::primed_t p) {
      return std::hash<::black::identifier>{}(p.label);
    }
  };
}

#endif // SYNTH_AUTOMATA_HPP