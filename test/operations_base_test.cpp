#include "exp/expected.h"

#include "arg.h"
#include "obj.h"
#include "obj_throw.h"
#include "state.h"

#include <utility>

#include <gtest/gtest.h>

using namespace exp::internal;

namespace {

using Base = expected_operations_base<Val, Err>;
using Base_void = expected_operations_base<void, Err>;

using B_t_throw = expected_operations_base<Val_throw, Err>;
using B_e_throw = expected_operations_base<Val, Err_throw>;
using B_void_e_throw = expected_operations_base<void, Err_throw>;

using B_t_throw_2 = expected_operations_base<Val_throw_2, Err>;
using B_e_throw_2 = expected_operations_base<Val, Err_throw_2>;
using B_void_e_throw_2 = expected_operations_base<void, Err_throw_2>;

} // namespace

TEST(expected_operations_base, in_place_t_construct_destroy) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base b(uninit);
    b.construct(std::in_place, std::move(arg), 1);
    ASSERT_EQ(b.val_.x, 1);
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::constructed);
    ASSERT_EQ(Err::s, State::none);
    b.destroy(std::in_place);
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  ASSERT_EQ(Val::s, State::destructed);
  ASSERT_EQ(Err::s, State::none);
  Val::reset();
  // T is void
  {
    Base_void b(uninit);
    b.construct(std::in_place);
    (void)b.dummy_;
    ASSERT_TRUE(b.has_val_);
    ASSERT_EQ(Err::s, State::none);
    b.destroy(std::in_place);
    ASSERT_EQ(Err::s, State::none);
  }
  ASSERT_EQ(Err::s, State::none);
}

TEST(expected_operations_base, unexpect_t_construct_destroy) {
  Val::reset();
  Err::reset();
  {
    Arg arg(1);
    Base b(uninit);
    b.construct(exp::unexpect, std::in_place, std::move(arg), 1);
    ASSERT_EQ(b.unexpect_.value().x, 1);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::constructed);
    b.destroy(exp::unexpect);
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  ASSERT_EQ(Val::s, State::none);
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
  // T is void
  {
    Arg arg(2);
    Base_void b(uninit);
    b.construct(exp::unexpect, std::in_place, std::move(arg), 2);
    ASSERT_EQ(b.unexpect_.value().x, 2);
    ASSERT_FALSE(b.has_val_);
    ASSERT_EQ(arg.x, -1);
    ASSERT_EQ(Err::s, State::constructed);
    b.destroy(exp::unexpect);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  ASSERT_EQ(Err::s, State::destructed);
  Err::reset();
}

TEST(expected_operations_base, copy_assign) {
  Val::reset();
  Err::reset();
  Val_throw::reset();
  Err_throw::reset();
  Val_throw_2::reset();
  Err_throw_2::reset();
  // this->has_value() && other.has_value()
  {
    Base other(std::in_place, 1);
    Val::reset();
    {
      Base b(std::in_place, 10);
      b.assign(other);
      ASSERT_EQ(Val::s, State::copy_assigned);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.val_.x, 1);
      ASSERT_EQ(other.val_.x, 1);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  {
    B_t_throw other(std::in_place, 2);
    Val_throw::reset();
    {
      B_t_throw b(std::in_place, 20);
      bool did_throw = false;
      try {
        Val_throw::t = May_throw::do_throw;
        b.assign(other);
      } catch (...) {
        ASSERT_EQ(Val_throw::s, State::copy_assigned); // failed
        ASSERT_EQ(Err::s, State::none);
        did_throw = true;
        Val_throw::t = May_throw::do_not_throw;
      }
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.val_.x, 2); // No exception safety guarantee.
      ASSERT_EQ(other.val_.x, 2);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Val_throw::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_throw::reset();
  }
  Val_throw::reset();
  // this->has_value() && !other.has_value() via
  // std::is_nothrow_copy_constructible_v<E>
  {
    Base other(exp::unexpect, 3);
    Err::reset();
    {
      Base b(std::in_place, 30);
      b.assign(other);
      ASSERT_EQ(Val::s, State::destructed);
      ASSERT_EQ(Err::s, State::copy_constructed);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 3);
      ASSERT_EQ(other.unexpect_.value().x, 3);
      Val::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  // this->has_value() && !other.has_value() via
  // std::is_nothrow_move_constructible_v<E>
  {
    B_e_throw other(exp::unexpect, 4);
    Err_throw::reset();
    {
      B_e_throw b(std::in_place, 40);
      b.assign(other);
      ASSERT_EQ(Val::s, State::destructed);
      // copy_constructed (tmp), move_constructed (this), destructed (tmp)
      ASSERT_EQ(Err_throw::s, State::destructed);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 4);
      ASSERT_EQ(other.unexpect_.value().x, 4);
      Err_throw::s = State::copy_constructed;
      Val::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_throw::s, State::destructed);
    Err_throw::reset();
  }
  Err_throw::reset();
  {
    B_e_throw other(exp::unexpect, 5);
    Err_throw::reset();
    {
      B_e_throw b(std::in_place, 50);
      bool did_throw = false;
      try {
        Err_throw::t = May_throw::do_throw;
        b.assign(other);
      } catch (...) {
        ASSERT_EQ(Val::s, State::constructed);
        ASSERT_EQ(Err_throw::s, State::copy_constructed); // failed
        did_throw = true;
        Err_throw::t = May_throw::do_not_throw;
      }
      ASSERT_TRUE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.val_.x, 50);
      ASSERT_EQ(other.unexpect_.value().x, 5);
      ASSERT_TRUE(did_throw);
      Err_throw::reset();
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err_throw::s, State::none);
    Val::reset();
  }
  Err_throw::reset();
  // this->has_value() && !other.has_value() via
  // std::is_nothrow_move_constructible_v<T>
  {
    B_e_throw_2 other(exp::unexpect, 6);
    Err_throw_2::reset();
    {
      B_e_throw_2 b(std::in_place, 60);
      b.assign(other);
      ASSERT_EQ(Val::s, State::destructed);
      ASSERT_EQ(Err_throw_2::s, State::copy_constructed);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 6);
      ASSERT_EQ(other.unexpect_.value().x, 6);
      Val::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_throw_2::s, State::destructed);
    Err_throw_2::reset();
  }
  Err_throw_2::reset();
  {
    B_e_throw_2 other(exp::unexpect, 7);
    Err_throw_2::reset();
    {
      B_e_throw_2 b(std::in_place, 70);
      bool did_throw = false;
      try {
        Err_throw_2::t = May_throw::do_throw;
        b.assign(other);
      } catch (...) {
        // move_constructed (tmp), move_constructed (this), destructed (tmp)
        ASSERT_EQ(Val::s, State::destructed);
        ASSERT_EQ(Err_throw_2::s, State::copy_constructed); // failed
        did_throw = true;
        Err_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_TRUE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.val_.x, 70);
      ASSERT_EQ(other.unexpect_.value().x, 7);
      ASSERT_TRUE(did_throw);
      Val::s = State::constructed;
      Err_throw_2::reset();
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err_throw_2::s, State::none);
    Val::reset();
  }
  Err_throw_2::reset();
  // !this->has_value() && !other.has_value()
  {
    Base other(exp::unexpect, 1);
    Err::reset();
    {
      Base b(exp::unexpect, 10);
      b.assign(other);
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::copy_assigned);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 1);
      ASSERT_EQ(other.unexpect_.value().x, 1);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    B_e_throw other(exp::unexpect, 2);
    Err_throw::reset();
    {
      B_e_throw b(exp::unexpect, 20);
      bool did_throw = false;
      try {
        Err_throw::t = May_throw::do_throw;
        b.assign(other);
      } catch (...) {
        ASSERT_EQ(Val::s, State::none);
        ASSERT_EQ(Err_throw::s, State::copy_assigned); // failed
        did_throw = true;
        Err_throw::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 2); // No exception safety guarantee.
      ASSERT_EQ(other.unexpect_.value().x, 2);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_throw::s, State::destructed);
    Err_throw::reset();
  }
  Err_throw::reset();
  // !this->has_value() && other.has_value() via
  // std::is_nothrow_copy_constructible_v<T>
  {
    Base other(std::in_place, 3);
    Val::reset();
    {
      Base b(exp::unexpect, 30);
      b.assign(other);
      ASSERT_EQ(Val::s, State::copy_constructed);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.val_.x, 3);
      ASSERT_EQ(other.val_.x, 3);
      Err::reset();
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  // !this->has_value() && other.has_value() via
  // std::is_nothrow_move_constructible_v<T>
  {
    B_t_throw other(std::in_place, 4);
    Val_throw::reset();
    {
      B_t_throw b(exp::unexpect, 40);
      b.assign(other);
      // copy_constructed (tmp), move_constructed (this), destructed (tmp)
      ASSERT_EQ(Val_throw::s, State::destructed);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.val_.x, 4);
      ASSERT_EQ(other.val_.x, 4);
      Val_throw::s = State::copy_constructed;
      Err::reset();
    }
    ASSERT_EQ(Val_throw::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_throw::reset();
  }
  Val_throw::reset();
  {
    B_t_throw other(std::in_place, 5);
    Val_throw::reset();
    {
      B_t_throw b(exp::unexpect, 50);
      bool did_throw = false;
      try {
        Val_throw::t = May_throw::do_throw;
        b.assign(other);
      } catch (...) {
        ASSERT_EQ(Val_throw::s, State::copy_constructed); // failed
        ASSERT_EQ(Err::s, State::constructed);
        did_throw = true;
        Val_throw::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 50);
      ASSERT_EQ(other.val_.x, 5);
      ASSERT_TRUE(did_throw);
      Val_throw::reset();
    }
    ASSERT_EQ(Val_throw::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Val_throw::reset();
  // !this->has_value() && other.has_value() via
  // std::is_nothrow_move_constructible_v<E>
  {
    B_t_throw_2 other(std::in_place, 6);
    Val_throw_2::reset();
    {
      B_t_throw_2 b(exp::unexpect, 60);
      b.assign(other);
      ASSERT_EQ(Val_throw_2::s, State::copy_constructed);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.val_.x, 6);
      ASSERT_EQ(other.val_.x, 6);
      Err::reset();
    }
    ASSERT_EQ(Val_throw_2::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_throw_2::reset();
  }
  Val_throw_2::reset();
  {
    B_t_throw_2 other(std::in_place, 7);
    Val_throw_2::reset();
    {
      B_t_throw_2 b(exp::unexpect, 70);
      bool did_throw = false;
      try {
        Val_throw_2::t = May_throw::do_throw;
        b.assign(other);
      } catch (...) {
        ASSERT_EQ(Val_throw_2::s, State::copy_constructed); // failed
        // move_constructed (tmp), move_constructed (this), destructed (tmp)
        ASSERT_EQ(Err::s, State::destructed);
        did_throw = true;
        Val_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 70);
      ASSERT_EQ(other.val_.x, 7);
      ASSERT_TRUE(did_throw);
      Val_throw_2::reset();
      Err::s = State::constructed;
    }
    ASSERT_EQ(Val_throw_2::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Val_throw_2::reset();
}

TEST(expected_operations_base, move_assign) {
  Val::reset();
  Err::reset();
  Val_throw::reset();
  Err_throw::reset();
  Val_throw_2::reset();
  Err_throw_2::reset();
  // this->has_value() && other.has_value()
  {
    Base other(std::in_place, 1);
    Val::reset();
    {
      Base b(std::in_place, 10);
      b.assign(std::move(other));
      ASSERT_EQ(Val::s, State::move_assigned);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.val_.x, 1);
      ASSERT_EQ(other.val_.x, -2);
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  {
    B_t_throw_2 other(std::in_place, 2);
    Val_throw_2::reset();
    {
      B_t_throw_2 b(std::in_place, 20);
      bool did_throw = false;
      try {
        Val_throw_2::t = May_throw::do_throw;
        b.assign(std::move(other));
      } catch (...) {
        ASSERT_EQ(Val_throw_2::s, State::move_assigned); // failed
        ASSERT_EQ(Err::s, State::none);
        did_throw = true;
        Val_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.val_.x, 2); // No exception safety guarantee.
      ASSERT_EQ(other.val_.x, -2);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Val_throw_2::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_throw_2::reset();
  }
  Val_throw_2::reset();
  // this->has_value() && !other.has_value() via
  // std::is_nothrow_move_constructible_v<E>
  {
    Base other(exp::unexpect, 3);
    Err::reset();
    {
      Base b(std::in_place, 30);
      b.assign(std::move(other));
      ASSERT_EQ(Val::s, State::destructed);
      ASSERT_EQ(Err::s, State::move_constructed);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 3);
      ASSERT_EQ(other.unexpect_.value().x, -1);
      Val::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  // this->has_value() && !other.has_value() via
  // std::is_nothrow_move_constructible_v<T>
  {
    B_e_throw_2 other(exp::unexpect, 6);
    Err_throw_2::reset();
    {
      B_e_throw_2 b(std::in_place, 60);
      b.assign(std::move(other));
      ASSERT_EQ(Val::s, State::destructed);
      ASSERT_EQ(Err_throw_2::s, State::move_constructed);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 6);
      ASSERT_EQ(other.unexpect_.value().x, -1);
      Val::reset();
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_throw_2::s, State::destructed);
    Err_throw_2::reset();
  }
  Err_throw_2::reset();
  {
    B_e_throw_2 other(exp::unexpect, 7);
    Err_throw_2::reset();
    {
      B_e_throw_2 b(std::in_place, 70);
      bool did_throw = false;
      try {
        Err_throw_2::t = May_throw::do_throw;
        b.assign(std::move(other));
      } catch (...) {
        // move_constructed (tmp), move_constructed (this), destructed (tmp)
        ASSERT_EQ(Val::s, State::destructed);
        ASSERT_EQ(Err_throw_2::s, State::move_constructed); // failed
        did_throw = true;
        Err_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_TRUE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.val_.x, 70);
      ASSERT_EQ(other.unexpect_.value().x, -1);
      ASSERT_TRUE(did_throw);
      Val::s = State::constructed;
      Err_throw_2::reset();
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err_throw_2::s, State::none);
    Val::reset();
  }
  Err_throw_2::reset();
  // !this->has_value() && !other.has_value()
  {
    Base other(exp::unexpect, 1);
    Err::reset();
    {
      Base b(exp::unexpect, 10);
      b.assign(std::move(other));
      ASSERT_EQ(Val::s, State::none);
      ASSERT_EQ(Err::s, State::move_assigned);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 1);
      ASSERT_EQ(other.unexpect_.value().x, -2);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    B_e_throw_2 other(exp::unexpect, 2);
    Err_throw_2::reset();
    {
      B_e_throw_2 b(exp::unexpect, 20);
      bool did_throw = false;
      try {
        Err_throw_2::t = May_throw::do_throw;
        b.assign(std::move(other));
      } catch (...) {
        ASSERT_EQ(Val::s, State::none);
        ASSERT_EQ(Err_throw_2::s, State::move_assigned); // failed
        did_throw = true;
        Err_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 2); // No exception safety guarantee.
      ASSERT_EQ(other.unexpect_.value().x, -2);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Val::s, State::none);
    ASSERT_EQ(Err_throw_2::s, State::destructed);
    Err_throw_2::reset();
  }
  Err_throw_2::reset();
  // !this->has_value() && other.has_value() via
  // std::is_nothrow_move_constructible_v<T>
  {
    Base other(std::in_place, 3);
    Val::reset();
    {
      Base b(exp::unexpect, 30);
      b.assign(std::move(other));
      ASSERT_EQ(Val::s, State::move_constructed);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.val_.x, 3);
      ASSERT_EQ(other.val_.x, -1);
      Err::reset();
    }
    ASSERT_EQ(Val::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val::reset();
  }
  Val::reset();
  // !this->has_value() && other.has_value() via
  // std::is_nothrow_move_constructible_v<E>
  {
    B_t_throw_2 other(std::in_place, 6);
    Val_throw_2::reset();
    {
      B_t_throw_2 b(exp::unexpect, 60);
      b.assign(std::move(other));
      ASSERT_EQ(Val_throw_2::s, State::move_constructed);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.val_.x, 6);
      ASSERT_EQ(other.val_.x, -1);
      Err::reset();
    }
    ASSERT_EQ(Val_throw_2::s, State::destructed);
    ASSERT_EQ(Err::s, State::none);
    Val_throw_2::reset();
  }
  Val_throw_2::reset();
  {
    B_t_throw_2 other(std::in_place, 7);
    Val_throw_2::reset();
    {
      B_t_throw_2 b(exp::unexpect, 70);
      bool did_throw = false;
      try {
        Val_throw_2::t = May_throw::do_throw;
        b.assign(std::move(other));
      } catch (...) {
        ASSERT_EQ(Val_throw_2::s, State::move_constructed); // failed
        // move_constructed (tmp), move_constructed (this), destructed (tmp)
        ASSERT_EQ(Err::s, State::destructed);
        did_throw = true;
        Val_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 70);
      ASSERT_EQ(other.val_.x, -1);
      ASSERT_TRUE(did_throw);
      Val_throw_2::reset();
      Err::s = State::constructed;
    }
    ASSERT_EQ(Val_throw_2::s, State::none);
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Val_throw_2::reset();
}

TEST(expected_operations_base, copy_assign_void) {
  Err::reset();
  Err_throw::reset();
  // this->has_value() && other.has_value()
  {
    Base_void other(std::in_place);
    {
      Base_void b(std::in_place);
      b.assign(other);
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      (void)b.dummy_;
      (void)other.dummy_;
    }
    ASSERT_EQ(Err::s, State::none);
  }
  // this->has_value() && !other.has_value()
  {
    Base_void other(exp::unexpect, 1);
    Err::reset();
    {
      Base_void b(std::in_place);
      b.assign(other);
      ASSERT_EQ(Err::s, State::copy_constructed);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 1);
      ASSERT_EQ(other.unexpect_.value().x, 1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    B_void_e_throw other(exp::unexpect, 2);
    Err_throw::reset();
    {
      B_void_e_throw b(std::in_place);
      bool did_throw = false;
      try {
        Err_throw::t = May_throw::do_throw;
        b.assign(other);
      } catch (...) {
        ASSERT_EQ(Err_throw::s, State::copy_constructed); // failed
        did_throw = true;
        Err_throw::t = May_throw::do_not_throw;
      }
      ASSERT_TRUE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      (void)b.dummy_;
      ASSERT_EQ(other.unexpect_.value().x, 2);
      ASSERT_TRUE(did_throw);
      Err_throw::reset();
    }
    ASSERT_EQ(Err_throw::s, State::none);
  }
  Err_throw::reset();
  // !this->has_value() && !other.has_value()
  {
    Base_void other(exp::unexpect, 3);
    Err::reset();
    {
      Base_void b(exp::unexpect, 30);
      b.assign(other);
      ASSERT_EQ(Err::s, State::copy_assigned);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 3);
      ASSERT_EQ(other.unexpect_.value().x, 3);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    B_void_e_throw other(exp::unexpect, 4);
    Err_throw::reset();
    {
      B_void_e_throw b(exp::unexpect, 40);
      bool did_throw = false;
      try {
        Err_throw::t = May_throw::do_throw;
        b.assign(other);
      } catch (...) {
        ASSERT_EQ(Err_throw::s, State::copy_assigned); // failed
        did_throw = true;
        Err_throw::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 4); // No exception safety guarantee.
      ASSERT_EQ(other.unexpect_.value().x, 4);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Err_throw::s, State::destructed);
    Err_throw::reset();
  }
  Err_throw::reset();
  // !this->has_value() && other.has_value()
  {
    Base_void other(std::in_place);
    {
      Base_void b(exp::unexpect, 50);
      b.assign(other);
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      (void)b.dummy_;
      (void)other.dummy_;
      Err::reset();
    }
    ASSERT_EQ(Err::s, State::none);
  }
}

TEST(expected_operations_base, move_assign_void) {
  Err::reset();
  Err_throw::reset();
  Err_throw_2::reset();
  // this->has_value() && other.has_value()
  {
    Base_void other(std::in_place);
    {
      Base_void b(std::in_place);
      b.assign(std::move(other));
      ASSERT_EQ(Err::s, State::none);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      (void)b.dummy_;
      (void)other.dummy_;
    }
    ASSERT_EQ(Err::s, State::none);
  }
  // this->has_value() && !other.has_value()
  {
    Base_void other(exp::unexpect, 1);
    Err::reset();
    {
      Base_void b(std::in_place);
      b.assign(std::move(other));
      ASSERT_EQ(Err::s, State::move_constructed);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 1);
      ASSERT_EQ(other.unexpect_.value().x, -1);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    B_void_e_throw_2 other(exp::unexpect, 2);
    Err_throw_2::reset();
    {
      B_void_e_throw_2 b(std::in_place);
      bool did_throw = false;
      try {
        Err_throw_2::t = May_throw::do_throw;
        b.assign(std::move(other));
      } catch (...) {
        ASSERT_EQ(Err_throw_2::s, State::move_constructed); // failed
        did_throw = true;
        Err_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_TRUE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      (void)b.dummy_;
      ASSERT_EQ(other.unexpect_.value().x, -1);
      ASSERT_TRUE(did_throw);
      Err_throw_2::reset();
    }
    ASSERT_EQ(Err_throw_2::s, State::none);
  }
  Err_throw_2::reset();
  // !this->has_value() && !other.has_value()
  {
    Base_void other(exp::unexpect, 3);
    Err::reset();
    {
      Base_void b(exp::unexpect, 30);
      b.assign(std::move(other));
      ASSERT_EQ(Err::s, State::move_assigned);
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 3);
      ASSERT_EQ(other.unexpect_.value().x, -2);
    }
    ASSERT_EQ(Err::s, State::destructed);
    Err::reset();
  }
  Err::reset();
  {
    B_void_e_throw_2 other(exp::unexpect, 4);
    Err_throw_2::reset();
    {
      B_void_e_throw_2 b(exp::unexpect, 40);
      bool did_throw = false;
      try {
        Err_throw_2::t = May_throw::do_throw;
        b.assign(std::move(other));
      } catch (...) {
        ASSERT_EQ(Err_throw_2::s, State::move_assigned); // failed
        did_throw = true;
        Err_throw_2::t = May_throw::do_not_throw;
      }
      ASSERT_FALSE(b.has_val_);
      ASSERT_FALSE(other.has_val_);
      ASSERT_EQ(b.unexpect_.value().x, 4); // No exception safety guarantee.
      ASSERT_EQ(other.unexpect_.value().x, -2);
      ASSERT_TRUE(did_throw);
    }
    ASSERT_EQ(Err_throw_2::s, State::destructed);
    Err_throw_2::reset();
  }
  Err_throw_2::reset();
  // !this->has_value() && other.has_value()
  {
    Base_void other(std::in_place);
    {
      Base_void b(exp::unexpect, 50);
      b.assign(std::move(other));
      ASSERT_EQ(Err::s, State::destructed);
      ASSERT_TRUE(b.has_val_);
      ASSERT_TRUE(other.has_val_);
      (void)b.dummy_;
      (void)other.dummy_;
      Err::reset();
    }
    ASSERT_EQ(Err::s, State::none);
  }
}
