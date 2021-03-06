/*==================================================================================================
  Copyright (c) 2015 QuasarDB
  Copyright (c) 2015 NumScale

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
=================================================================================================**/
#pragma once

#include <brigand/functions/lambda/apply.hpp>

namespace brigand { namespace detail
{

  template<class Functor, class State, class Sequence>
  struct fold_impl
  {
      using type = State;
  };

  template<
      class Functor, class State, template <class...> class Sequence,
      class T0>
  struct fold_impl<Functor, State, Sequence<T0>>
  {
      using type = brigand::apply<Functor, State, T0>;
  };

  template<
      class Functor, class State, template <class...> class Sequence,
      class T0, class T1>
  struct fold_impl<Functor, State, Sequence<T0, T1>>
  {
      using type = brigand::apply<Functor,
          brigand::apply<Functor,State, T0>, T1
      >;
  };

  template<
      class Functor, class State, template <class...> class Sequence,
      class T0, class T1, class T2>
  struct fold_impl<Functor, State, Sequence<T0, T1, T2>>
  {
      using type = brigand::apply<Functor,
          brigand::apply<Functor,
              brigand::apply<Functor, State, T0>, T1
          >, T2
      >;
  };

  template<
      class Functor, class State, template <class...> class Sequence,
      class T0, class T1, class T2, class T3>
  struct fold_impl<Functor, State, Sequence<T0, T1, T2, T3>>
  {
      using type = brigand::apply<Functor,
          brigand::apply<Functor,
              brigand::apply<Functor,
                  brigand::apply<Functor, State, T0>, T1
              >, T2
          >, T3
      >;
  };

  template<
      class Functor, class State, template <class...> class Sequence,
      class T0, class T1, class T2, class T3, class T4>
  struct fold_impl<Functor, State, Sequence<T0, T1, T2, T3, T4>>
  {
      using type = brigand::apply<Functor,
          brigand::apply<Functor,
              brigand::apply<Functor,
                  brigand::apply<Functor,
                      brigand::apply<Functor, State, T0>, T1
                  >, T2
              >, T3
          >, T4
      >;
  };

  template<
      class Functor, class State, template <class...> class Sequence,
      class T0, class T1, class T2, class T3, class T4, class T5>
  struct fold_impl<Functor, State, Sequence<T0, T1, T2, T3, T4, T5>>
  {
      using type = brigand::apply<Functor,
          brigand::apply<Functor,
              brigand::apply<Functor,
                  brigand::apply<Functor,
                      brigand::apply<Functor,
                          brigand::apply<Functor, State, T0>, T1
                      >, T2
                  >, T3
              >, T4
          >, T5
      >;
  };

  template<
      class Functor, class State, template <class...> class Sequence,
      class T0, class T1, class T2, class T3, class T4, class T5, class T6>
  struct fold_impl<Functor, State, Sequence<T0, T1, T2, T3, T4, T5, T6>>
  {
      using type = brigand::apply<Functor,
          brigand::apply<Functor,
              brigand::apply<Functor,
                  brigand::apply<Functor,
                      brigand::apply<Functor,
                          brigand::apply<Functor,
                              brigand::apply<Functor, State, T0>, T1
                          >, T2
                      >, T3
                  >, T4
              >, T5
          >, T6
      >;
  };

  template<
      class Functor, class State, template <class...> class Sequence,
      class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
  struct fold_impl<Functor, State, Sequence<T0, T1, T2, T3, T4, T5, T6, T7>>
  {
      using type = brigand::apply<Functor,
          brigand::apply<Functor,
              brigand::apply<Functor,
                  brigand::apply<Functor,
                      brigand::apply<Functor,
                          brigand::apply<Functor,
                              brigand::apply<Functor,
                                  brigand::apply<Functor, State, T0>, T1
                              >, T2
                          >, T3
                      >, T4
                  >, T5
              >, T6
          >, T7
      >;
  };

  template<
      class Functor, class State, template <class...> class Sequence,
      class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class... T>
  struct fold_impl<Functor, State, Sequence<T0, T1, T2, T3, T4, T5, T6, T7, T...>>
  : fold_impl<
      Functor,
      brigand::apply<Functor,
          brigand::apply<Functor,
              brigand::apply<Functor,
                  brigand::apply<Functor,
                      brigand::apply<Functor,
                          brigand::apply<Functor,
                              brigand::apply<Functor,
                                  brigand::apply<Functor,
                                      State, T0
                                  >, T1
                              >, T2
                          >, T3
                      >, T4
                  >, T5
              >, T6
          >, T7
      >,
      Sequence<T...>
  >
  {};

} }
