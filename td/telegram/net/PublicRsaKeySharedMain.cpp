//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2024
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "td/telegram/net/PublicRsaKeySharedMain.h"

#include "td/utils/format.h"
#include "td/utils/logging.h"
#include "td/utils/Slice.h"
#include "td/utils/SliceBuilder.h"

#include <algorithm>

namespace td {

PublicRsaKeySharedMain::PublicRsaKeySharedMain(bool is_test) {
  auto add_pem = [this](CSlice pem) {
    auto r_rsa = mtproto::RSA::from_pem_public_key(pem);
    LOG_CHECK(r_rsa.is_ok()) << r_rsa.error() << " " << pem;

    if (r_rsa.is_ok()) {
      auto rsa = r_rsa.move_as_ok();
      auto fingerprint = rsa.get_fingerprint();
      if (get_rsa_key_unsafe(fingerprint) != nullptr) {
        return;
      }
      keys_.push_back(RsaKey{std::move(rsa), fingerprint});
    }
  };

  if (is_test) {
    add_pem(
        "-----BEGIN RSA PUBLIC KEY-----\n"
        "MIIBCgKCAQEAyMEdY1aR+sCR3ZSJrtztKTKqigvO/vBfqACJLZtS7QMgCGXJ6XIR\n"
        "yy7mx66W0/sOFa7/1mAZtEoIokDP3ShoqF4fVNb6XeqgQfaUHd8wJpDWHcR2OFwv\n"
        "plUUI1PLTktZ9uW2WE23b+ixNwJjJGwBDJPQEQFBE+vfmH0JP503wr5INS1poWg/\n"
        "j25sIWeYPHYeOrFp/eXaqhISP6G+q2IeTaWTXpwZj4LzXq5YOpk4bYEQ6mvRq7D1\n"
        "aHWfYmlEGepfaYR8Q0YqvvhYtMte3ITnuSJs171+GDqpdKcSwHnd6FudwGO4pcCO\n"
        "j4WcDuXc2CTHgH8gFTNhp/Y8/SpDOhvn9QIDAQAB\n"
        "-----END RSA PUBLIC KEY-----");
    return;
  }

  add_pem(
      "-----BEGIN RSA PUBLIC KEY-----\n"
      "MIIBCgKCAQEA6LszBcC1LGzyr992NzE0ieY+BSaOW622Aa9Bd4ZHLl+TuFQ4lo4g\n"
      "5nKaMBwK/BIb9xUfg0Q29/2mgIR6Zr9krM7HjuIcCzFvDtr+L0GQjae9H0pRB2OO\n"
      "62cECs5HKhT5DZ98K33vmWiLowc621dQuwKWSQKjWf50XYFw42h21P2KXUGyp2y/\n"
      "+aEyZ+uVgLLQbRA1dEjSDZ2iGRy12Mk5gpYc397aYp438fsJoHIgJ2lgMv5h7WY9\n"
      "t6N/byY9Nw9p21Og3AoXSL2q/2IJ1WRUhebgAdGVMlV1fkuOQoEzR7EdpqtQD9Cs\n"
      "5+bfo3Nhmcyvk5ftB0WkJ9z6bNZ7yxrP8wIDAQAB\n"
      "-----END RSA PUBLIC KEY-----");
}

Result<mtproto::PublicRsaKeyInterface::RsaKey> PublicRsaKeySharedMain::get_rsa_key(const vector<int64> &fingerprints) {
  for (auto fingerprint : fingerprints) {
    const auto *rsa_key = get_rsa_key_unsafe(fingerprint);
    if (rsa_key != nullptr) {
      return RsaKey{rsa_key->rsa.clone(), fingerprint};
    }
  }
  return Status::Error(PSLICE() << "Unknown fingerprints " << format::as_array(fingerprints));
}

void PublicRsaKeySharedMain::drop_keys() {
  // nothing to do
}

const mtproto::PublicRsaKeyInterface::RsaKey *PublicRsaKeySharedMain::get_rsa_key_unsafe(int64 fingerprint) const {
  auto it = std::find_if(keys_.begin(), keys_.end(),
                         [fingerprint](const auto &value) { return value.fingerprint == fingerprint; });
  if (it == keys_.end()) {
    return nullptr;
  }
  return &*it;
}

}  // namespace td
