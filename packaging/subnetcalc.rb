class Subnetcalc < Formula
  desc "IPv4/IPv6 Subnet Calculator"
  homepage "https://www.nntb.no/~dreibh/subnetcalc/"
  url "https://www.nntb.no/~dreibh/subnetcalc/download/subnetcalc-2.7.6.tar.xz"
  sha256 "dcdca4fe1f402c69c4dee1adc6ae1b0c133de8726f509560a1072d54d9ae2c72"
  license "GPL-3.0-or-later"
  head "https://github.com/dreibh/subnetcalc.git", branch: "master"

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
    with_env(LC_ALL: "C") do
      # 1. Version sanity check:
      assert_match(/SubNetCalc\s+#{Regexp.escape(version)}/, shell_output("#{bin}/subnetcalc -v"))

      # 2. Offline IPv4 calculation:
      output_v4 = shell_output("#{bin}/subnetcalc 192.168.1.1/24 -c -n -g")
      assert_match(/Network\s+=\s+192\.168\.1\.0\s*\/\s*24/, output_v4)
      assert_match(/Netmask\s+=\s+255\.255\.255\.0/, output_v4)
      assert_match(/Broadcast\s+=\s+192\.168\.1\.255/, output_v4)

      # 3. Offline IPv6 calculation:
      output_v6 = shell_output("#{bin}/subnetcalc 2001:db8::1/64 -c -n -g")
      assert_match(/Network\s+=\s+2001:db8::\s*\/\s*64/, output_v6)
      assert_match(/Netmask\s+=\s+ffff:ffff:ffff:ffff:/, output_v6)
      assert_match(/2001:db8::1\s+is a HOST address in\s+2001:db8::\/64/, output_v6)
    end
  end
end
