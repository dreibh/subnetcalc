Name: subnetcalc
Version: 2.2.0~rc1
Release: 1%{?dist}
Summary: IPv4/IPv6 Subnet Calculator
Group: Converted/net
License: GPLv3
Source: %{name}-%{version}.tar.gz
URL: http://www.iem.uni-due.de/~dreibh/subnetcalc/

BuildRequires: GeoIP-devel
Requires: GeoIP

%define _unpackaged_files_terminate_build 0

%description
SubNetCalc is an IPv4/IPv6 subnet address calculator. For given IPv4 or
IPv6 address and netmask or prefix length, it calculates network address,
broadcast address, maximum number of hosts and host address range. The
output is colourized for better readability (e.g. network part, host part).
Also, it prints the addresses in binary format for better understandability.
Furthermore, it can identify the address type (e.g. multicast, unique local,
site local, etc.) and extract additional information from the address
(e.g. type, scope, interface ID, etc.). Finally, it can generate
IPv6 unique local prefixes.

%prep
%setup -q


%build
%configure
make %{?_smp_mflags}

%install
make DESTDIR=%{buildroot} install

%files
"/usr/bin/subnetcalc"
"/usr/share/man/man1/subnetcalc.1.gz"

%doc

%changelog
* Thu Sep 26 2013 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.1.5~rc1-1
- Created RPM package.
