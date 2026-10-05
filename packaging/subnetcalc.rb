class Subnetcalc < Formula
  desc "IPv4/IPv6 Subnet Calculator"
  homepage "https://www.nntb.no/~dreibh/subnetcalc/"
  url "https://www.nntb.no/~dreibh/subnetcalc/download/subnetcalc-2.7.5.tar.xz"
  sha256 "aa0a006c2dc6cb2de890a79971fbd5fda26a531a77addd1c6644c49e9f3ef4f5"
  license "GPL-3.0-or-later"

  depends_on "cmake" => :build
  depends_on "gettext"
  depends_on "libidn2"
  depends_on "libmaxminddb"

  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    system "#{bin}/subnetcalc", "192.168.1.1/24"
  end
end
