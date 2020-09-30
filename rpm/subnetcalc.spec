Name: subnetcalc
Version: 2.4.16
Release: 1
Summary: IPv4/IPv6 Subnet Calculator
Group: Applications/Internet
License: GPL-3+
URL: https://www.uni-due.de/~be0001/subnetcalc/
Source: https://www.uni-due.de/~be0001/subnetcalc/download/%{name}-%{version}.tar.xz

AutoReqProv: on
BuildRequires: cmake
BuildRequires: gcc
BuildRequires: gcc-c++
BuildRequires: GeoIP-devel
Requires: GeoIP
BuildRoot: %{_tmppath}/%{name}-%{version}-build

%define _unpackaged_files_terminate_build 0

%description
SubNetCalc is an IPv4/IPv6 subnet address calculator. For given IPv4 or IPv6 address and netmask or prefix length, it calculates network address, broadcast address, maximum number of hosts and host address range. The output is colourized for better readability (e.g. network part, host part). Also, it prints the addresses in binary format for better understandability. Furthermore, it can identify the address type (e.g. multicast, unique local, site local, etc.) and extract additional information from the address (e.g. type, scope, interface ID, etc.). Finally, it can generate IPv6 unique local prefixes.

%prep
%setup -q

%build
%cmake -DCMAKE_INSTALL_PREFIX=/usr .
%cmake_build

%install
%cmake_install

%files
%{_bindir}/subnetcalc
%{_mandir}/man1/subnetcalc.1.gz

%doc

%changelog
* Mon May 18 2020 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.4.16
- New upstream release.
* Fri Feb 07 2020 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.4.15
- New upstream release.
* Wed Aug 14 2019 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.4.14
- New upstream release.
* Wed Aug 07 2019 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.4.13
- New upstream release.
* Fri Jul 26 2019 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.4.12
- New upstream release.
* Tue May 21 2019 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.4.11
- New upstream release.
* Wed Nov 22 2017 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.2.0
- Created RPM package.
