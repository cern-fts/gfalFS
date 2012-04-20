Name:				gfalFS
Version:			1.0.0
Release:			2beta1%{?dist}
Summary:			Mount and Unmount a GFAL file system
Group:				Applications/Internet
License:			ASL 2.0
URL:				https://svnweb.cern.ch/trac/lcgutil/wiki/gfal2
# svn export http://svn.cern.ch/guest/lcgutil/wlcggridfs/trunk gfalfs
Source0:			http://grid-deployment.web.cern.ch/grid-deployment/dms/lcgutil/tar/%{name}/%{name}-%{version}.tar.gz 
BuildRoot:			%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:		cmake
BuildRequires:		glib2-devel
BuildRequires:		gfal2-devel
BuildRequires:		fuse-devel

Requires:			fuse%{?_isa}

%description
gfalFS provides a solution to mount any distributed \
file system managed by GFAL 2.0 permitting easy \
interactions with a large set of distributed file systems. 

%clean
rm -rf "$RPM_BUILD_ROOT";
make clean

%prep
%setup -q

%build
%cmake -DDOC_INSTALL_DIR=%{_docdir}/%{name}-%{version} .
make %{?_smp_mflags}

%install
rm -rf "$RPM_BUILD_ROOT"; 
make %{?_smp_mflags} DESTDIR=$RPM_BUILD_ROOT install

%files
%defattr (-,root,root)
%{_bindir}/gfalFS
%{_bindir}/gfalFS_umount
%{_docdir}/%{name}-%{version}/DESCRIPTION
%{_docdir}/%{name}-%{version}/VERSION
%{_docdir}/%{name}-%{version}/LICENSE
%{_docdir}/%{name}-%{version}/README

%changelog
* Mon Nov 14 2011 Adrien Devress <adevress at cern.ch> - 1.0.0-2beta1
 - Initial gfalFS 1.0 preview release
