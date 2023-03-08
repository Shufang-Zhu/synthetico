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

#include <synthetico/automata.hpp>

#include <black/logic/prettyprint.hpp>

namespace synth {
  
  using namespace logic;

  struct encoder {

    static formula<pLTL> nnf(formula<pLTL> f);

    void collect(spec sp);

    static proposition ground(formula<pLTL> y);

    static formula<pLTL> lift(proposition p);

    static automata::formula snf(formula<pLTL> f);

    automata encode(spec sp);

    std::vector<yesterday<pLTL>> yreqs;
    std::vector<w_yesterday<pLTL>> zreqs;
    std::vector<proposition> variables;

  };

  automata encode(spec sp) {
    return encoder{}.encode(sp);
  }

  formula<pLTL> encoder::nnf(formula<pLTL> f) {
    return f.match(
      [](boolean b) { return b; },
      [](proposition p) { return p; },
      [](disjunction<pLTL>, auto left, auto right) {
        return nnf(left) || nnf(right);
      },
      [](conjunction<pLTL>, auto left, auto right) {
        return nnf(left) && nnf(right);
      },
      [](implication<pLTL>, auto left, auto right) {
        return nnf(!left || right);
      },
      [](iff<pLTL>, auto left, auto right) {
        return nnf(implies(left, right) && implies(right, left));
      },
      [](yesterday<pLTL>, auto arg) {
        return Y(nnf(arg));
      },
      [](w_yesterday<pLTL>, auto arg) {
        return Z(nnf(arg));
      },
      [](once<pLTL>, auto arg) {
        return O(nnf(arg));
      },
      [](historically<pLTL>, auto arg) {
        return H(nnf(arg));
      },
      [](since<pLTL>, auto left, auto right) {
        return S(nnf(left), nnf(right));
      },
      [](triggered<pLTL>, auto left, auto right) {
        return T(nnf(left), nnf(right));
      },
      [](negation<pLTL>, auto arg) {
        return arg.match(
          [](boolean b) { return b.sigma()->boolean(!b.value()); },
          [](proposition p) { return !p; },
          [](negation<pLTL>, auto op) { return op; },
          [](disjunction<pLTL>, auto left, auto right) {
            return nnf(!left) && nnf(!right);
          },
          [](conjunction<pLTL>, auto left, auto right) {
            return nnf(!left) || nnf(!right);
          },
          [](implication<pLTL>, auto left, auto right) {
            return nnf(left) && nnf(!right);
          },
          [](iff<pLTL>, auto left, auto right) {
            return nnf(!implies(left,right)) || nnf(!implies(right,left));
          },
          [](yesterday<pLTL>, auto op) {
            return Z(nnf(!op));
          },
          [](w_yesterday<pLTL>, auto op) {
            return Y(nnf(!op));
          },
          [](once<pLTL>, auto op) {
            return H(nnf(!op));
          },
          [](historically<pLTL>, auto op) {
            return O(nnf(!op));
          },
          [](since<pLTL>, auto left, auto right) {
            return T(nnf(!left), nnf(!right));
          },
          [](triggered<pLTL>, auto left, auto right) {
            return S(nnf(!left), nnf(!right));
          }
        );
      }
    );
  }

  void encoder::collect(spec sp) {
    sp.type.match(
      [&](spec::type_t::eventually) {
        variables.push_back(ground(Y(sp.formula)));
        yreqs.push_back(Y(sp.formula));
      },
      [&](spec::type_t::always) {
        variables.push_back(ground(Z(sp.formula)));
        zreqs.push_back(Z(sp.formula));    
      }
    );    

    for_each_child_deep(sp.formula, [&](auto child) {
      child.match(
        [&](yesterday<pLTL> y) {
          variables.push_back(ground(y));
          yreqs.push_back(y);
        },
        [&](w_yesterday<pLTL> z) {
          variables.push_back(ground(z));
          zreqs.push_back(z);
        },
        [&](since<pLTL> s) {
          variables.push_back(ground(Y(s)));
          yreqs.push_back(Y(s));
        },
        [&](triggered<pLTL> s) {
          variables.push_back(ground(Z(s)));
          zreqs.push_back(Z(s));
        },
        [&](once<pLTL> o) {
          variables.push_back(ground(Y(o)));
          yreqs.push_back(Y(o));
        },
        [&](historically<pLTL> h) {
          variables.push_back(ground(Z(h)));
          zreqs.push_back(Z(h));
        },
        [](otherwise) { }
      );
    });
  }

  proposition encoder::ground(formula<pLTL> f) {
    return f.sigma()->proposition(f);
  }

  formula<pLTL> encoder::lift(proposition p) {
    auto name = p.name().to<formula<pLTL>>();
    black_assert(name);
    
    return *name;
  }
  
  automata::formula encoder::snf(formula<pLTL> f) {
    return f.match(
      [](boolean b) { return b; },
      [](proposition p) { return p; },
      [](negation<pLTL>, auto arg) { 
        return !snf(arg);
      },
      [](disjunction<pLTL>, auto left, auto right) {
        return snf(left) || snf(right);
      },
      [](conjunction<pLTL>, auto left, auto right) {
        return snf(left) && snf(right);
      },
      [](yesterday<pLTL> y) {
        return ground(y);
      },
      [](w_yesterday<pLTL> z) {
        return ground(z);
      },
      [](once<pLTL>, auto arg) {
        return snf(arg) || ground(Y(O(arg)));
      },
      [](historically<pLTL>, auto arg) {
        return snf(arg) && ground(Z(H(arg)));
      },
      [](since<pLTL>, auto left, auto right) {
        return snf(right) || (snf(left) && ground(Y(S(left, right))));
      },
      [](triggered<pLTL>, auto left, auto right) {
        return snf(right) && (snf(left) || ground(Z(T(left, right))));
      },
      [](implication<pLTL>) -> automata::formula { black_unreachable(); },
      [](iff<pLTL>) -> automata::formula { black_unreachable(); }
    );
  }

  automata encoder::encode(spec sp) {
    alphabet &sigma = *sp.formula.sigma();
    sp.formula = encoder::nnf(sp.formula);

    collect(sp);

    automata::formula init = 
      big_and(sigma, zreqs, [](auto req) {
        return ground(req);
      }) && 
      big_and(sigma, yreqs, [](auto req) {
        return !ground(req);
      });

    automata::formula trans = big_and(sigma, variables, [](proposition var) {
      auto req = lift(var).to<unary<pLTL>>();
      black_assert(req);
      
      return iff(primed(var), snf(req->argument()));
    });

    automata::formula objective = sp.type.match(
      [&](spec::type_t::eventually) {
        return ground(Y(sp.formula));
      },
      [&](spec::type_t::always) {
        return ground(Z(sp.formula));
      }
    );

    return automata{sp.inputs, sp.outputs, variables, init, trans, objective};
  }

  std::ostream &operator<<(std::ostream &str, automata aut) {
    str << "inputs:\n";
    for(auto in : aut.inputs) {
      str << "- " << to_string(in) << "\n";
    }
    
    str << "\noutputs:\n";
    for(auto out : aut.inputs) {
      str << "- " << to_string(out) << "\n";
    }

    str << "\ninit:\n";
    str << "- " << to_string(aut.init) << "\n";
    
    str << "\ntrans:\n";
    str << "- " << to_string(aut.trans) << "\n";
    
    str << "\nobjective:\n";
    str << "- " << to_string(aut.objective) << "\n";

    return str;
  }
}