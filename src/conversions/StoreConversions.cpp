/*
 * StoreConversions.cpp
 */

#include <softadastra/sdk/Key.hpp>
#include <softadastra/sdk/Value.hpp>

namespace softadastra::sdk
{
  namespace store_types = softadastra::store::types;

  store_types::Key Key::to_store() const
  {
    return store_types::Key{value_};
  }

  Key Key::from_store(const store_types::Key &key)
  {
    return Key{key.str()};
  }

  store_types::Value Value::to_store() const
  {
    return store_types::Value::from_bytes(data_);
  }

  Value Value::from_store(const store_types::Value &value)
  {
    return Value::from_bytes(value.bytes());
  }

} // namespace softadastra::sdk
