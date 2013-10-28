
Name:				gfalFS
Version:			1.4.0
Release:			2%{?dist}
Summary:			Filesystem client based on GFAL 2.0
Group:				Applications/Internet
License:			ASL 2.0
URL:				https://svnweb.cern.ch/trac/lcgutil/wiki/gfal2
# svn export http://svn.cern.ch/guest/lcgutil/gfalFS/trunk gfalFS
Source0:			http://grid-deployment.web.cern.ch/grid-deployment/dms/lcgutil/tar/%{name}/%{name}-%{version}.tar.gz
BuildRoot:			%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:		cmake
BuildRequires:		gfal2-devel
BuildRequires:		fuse-devel

Requires:			fuse%{?_isa}
Provides:                       gfal2-fuse = %{version}

%description
gfalFS is a filesystem based on FUSE capable of operating on remote storage
systems managed by GFAL 2.0. This include the common file access protocols 
in lcg ( SRM, GRIDFTP, DCAP, RFIO, LFN, ...). The practical effect is that
the user can seamlessly interact with grid and cloud storage systems just 
as if they were local files.

%clean
rm -rf %{buildroot};
make clean

%prep
%setup -q

%build
%cmake \
-DDOC_INSTALL_DIR=%{_docdir}/%{name}-%{version} .
make %{?_smp_mflags}

%install
rm -rf %{buildroot}; 
make DESTDIR=%{buildroot} install

%files
%defattr (-,root,root)
%{_bindir}/gfalFS
%{_bindir}/gfalFS_umount
%{_mandir}/man1/*
%{_docdir}/%{name}-%{version}/DESCRIPTION
%{_docdir}/%{name}-%{version}/VERSION
%{_docdir}/%{name}-%{version}/LICENSE
%{_docdir}/%{name}-%{version}/README

%changelog
* Mon Oct 28 2013 adevress at cern.ch - 1.4.0-2
 - Update 1.4.0 of gfalFS

* Wed Mar 20 2013 Adrien Devresse <adevress at cern.ch> - 1.2.0-0
 - fix a EIO problem with the gfal 2.0 http plugin 

* Thu Nov 29 2012 Adrien Devresse <adevress at cern.ch> - 1.0.1-0
 - fix a 32 bits off_t size problem with gfal 2.1


* Fri Jul 20 2012 Adrien Devresse <adevress at cern.ch> - 1.0.0-1
 - initial 1.0 release
 - include bug fix for srm and gsiftp url for fgettr

* Thu May 03 2012 Adrien Devresse <adevress at cern.ch> - 1.0.0-0.3.20120503010snap
 - bug correction with fgetattr on gsiftp / srm file system
 - minor changes applied from the fedora review comments

* Thu May 03 2012 Adrien Devresse <adevress at cern.ch> - 1.0.0-0.2.2012050202snap
 - improve global EPEL compliance.

* Mon Nov 14 2011 Adrien Devresse <adevress at cern.ch> - 1.0.0-0.2.2012041515snap
 - Initial gfalFS 1.0 preview release
