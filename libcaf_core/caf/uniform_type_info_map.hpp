/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2015                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#ifndef CAF_DETAIL_UNIFORM_TYPE_INFO_MAP_HPP
#define CAF_DETAIL_UNIFORM_TYPE_INFO_MAP_HPP

#include <set>
#include <map>
#include <string>
#include <utility>
#include <typeinfo>
#include <stdexcept>
#include <typeindex>
#include <type_traits>
#include <unordered_map>

#include "caf/fwd.hpp"

#include "caf/atom.hpp"
#include "caf/unit.hpp"
#include "caf/node_id.hpp"
#include "caf/duration.hpp"
#include "caf/system_messages.hpp"
#include "caf/type_erased_value.hpp"

#include "caf/type_nr.hpp"
#include "caf/detail/type_list.hpp"
#include "caf/detail/shared_spinlock.hpp"

namespace caf {

class uniform_type_info_map {
public:
  friend class actor_system;

  using value_factory = std::function<type_erased_value_ptr ()>;

  using actor_factory_result = std::pair<strong_actor_ptr, std::set<std::string>>;

  using actor_factory = std::function<actor_factory_result (actor_config&, message&)>;

  using actor_factories = std::unordered_map<std::string, actor_factory>;

  using value_factories_by_name = std::unordered_map<std::string, value_factory>;

  using value_factories_by_rtti = std::unordered_map<std::type_index, value_factory>;

  using value_factory_kvp = std::pair<std::string, value_factory>;

  using portable_names = std::unordered_map<std::type_index, std::string>;

  using error_renderer = std::function<std::string (uint8_t, atom_value, const message&)>;

  using error_renderers = std::unordered_map<atom_value, error_renderer>;

  type_erased_value_ptr make_value(uint16_t nr) const;

  type_erased_value_ptr make_value(const std::string& uniform_name) const;

  type_erased_value_ptr make_value(const std::type_info& ti) const;

  actor_factory_result make_actor(const std::string& name,
                                  actor_config& cfg,
                                  message& msg) const;

  /// Returns the portable name for given type information or `nullptr`
  /// if no mapping was found.
  const std::string* portable_name(uint16_t nr, const std::type_info* ti) const;

  /// Returns the portable name for given type information or `nullptr`
  /// if no mapping was found.
  inline const std::string*
  portable_name(const std::pair<uint16_t, const std::type_info*>& x) const {
    return portable_name(x.first, x.second);
  }

  error_renderer renderer(atom_value x) const;

  /// Returns the enclosing actor system.
  inline actor_system& system() const {
    return system_;
  }

private:
  uniform_type_info_map(actor_system& sys);

  actor_system& system_;

  // message types
  std::array<value_factory_kvp, type_nrs - 1> builtin_;
  value_factories_by_name custom_by_name_;
  value_factories_by_rtti custom_by_rtti_;
  value_factories_by_name ad_hoc_;
  mutable detail::shared_spinlock ad_hoc_mtx_;

  // message type names
  std::array<std::string, type_nrs - 1> builtin_names_;
  portable_names custom_names_;

  // actor types
  actor_factories factories_;

  // error types
  error_renderers error_renderers_;
};

} // namespace caf

#endif // CAF_DETAIL_UNIFORM_TYPE_INFO_MAP_HPP