# -*- ruby -*-
require 'rubygems'
require 'rake/clean'
require 'crxmake'
require 'net/github-upload'
require 'json'
$name     = "itunes_daemon"
$env      = File.open("manifest.json", "rb"){|f| JSON.parse f.read }
$manifest = "src/manifest.json"
$pem      = File.expand_path "~/dev/private/#{$name}.pem"
$manifest_data = File.open($manifest, 'rb'){|f| JSON.parse(f.read) }
$version  = $manifest_data["version"]

WIN       = RUBY_PLATFORM =~ /mswin(?!ce)|mingw|cygwin|bccwin/
LINUX     = !WIN && RUBY_PLATFORM =~ /linux/
MAC       = !WIN && !LINUX && RUBY_PLATFORM =~ /darwin/
OS        = (WIN)? :win : (LINUX)? :linux : (MAC)? :mac : :unknown

SRC       = FileList["**/*.cc"]

case OS
when :win
  TARGET = "ext/#{$name}.dll"
  CC     = "cl"
  CFLAGS = '/D "WIN32" /D "_WINDOWS" /D "XP_WIN32" /D "MOZILLA_STRICT_API" /D "XPCOM_GLUE" /D "XP_WIN" /D "_X86_" /D "_WINDLL" /D "_UNICODE" /D "UNICODE"  /EHsc /W3 /c  /Gd /TP'
  INCLUDE= "/I ./include"
  CLEAN.include(SRC.ext("obj"), SRC.ext("lib"), SRC.ext("ilk"), SRC.ext("res"), SRC.ext("exp"), TARGET)
  CLOBBER.include("src/#{$name}.dll")
when :linux, :mac
  CC     = "g++"
  CFLAGS = "-g -pedantic -Wall -O2 -DXULRUNNER_SDK -DXP_UNIX=1 -DMOZ_X11=1 -fPIC"
  INCLUDE= "-I/usr/include/xulrunner-1.9/stable -I./include"
  TARGET = "ext/#{$name}.so"
  CLEAN.include(SRC.ext("o"), TARGET)
end

# package task
namespace :pkg do
  desc "crx"
  task :crx => "src/#{$name}.dll" do
    mkdir_p "pkg" unless File.exist?("pkg")
    package = "pkg/#{$name}.crx"
    rm package if File.exist?(package)
    CrxMake.make(
      :ex_dir => "src",
      :pkey   => $pem,
      :crx_output => package,
      :verbose => true,
      :ignorefile => /\.swp$/,
      :ignoredir => /^\.(?:svn|git)$|^CVS$/
    )
  end

  desc "zip"
  task :zip do
    mkdir_p "pkg" unless File.exist?("pkg")
    package = "pkg/#{$name}.zip"
    rm package if File.exist?(package)
    CrxMake.zip(
      :ex_dir => "src",
      :pkey   => $pem,
      :zip_output => package,
      :verbose => true,
      :ignorefile => /\.swp$/,
      :ignoredir => /^\.(?:svn|git)$|^CVS$/
    )
  end
  directory "package"
end

desc "upload"
task :upload do
  login = `git config github.user`.chomp
  token = `git config github.token`.chomp
  repos = "iTunesDaemon"
  gh = Net::GitHub::Upload.new(
    :login => login,
    :token => token
  )
  direct_link = gh.replace(
    :repos => repos,
    :file  => "pkg/#{$name}.crx",
    :description => "latest version: #{$version}"
  ).gsub("%2F", "/")
  puts direct_link
  direct_link = gh.replace(
    :repos => repos,
    :file  => "updates.xml",
    :description => "updates.xml version: #{$version}"
  ).gsub("%2F", "/")
  puts direct_link
end

desc "compile"
task :compile => TARGET

case OS
when :win
  file "src/#{$name}.dll" => TARGET do |t|
    cp TARGET, "src/"
  end
  file TARGET => ["ext/#{$name}.cc", "ext/#{$name}.h", "ext/#{$name}.def", "ext/#{$name}.res"] do |t|
    sh "#{CC} #{CFLAGS} #{INCLUDE} -DVERSION=\"#{$version}\" #{t.prerequisites.first}"
    mv "#{$name}.obj", "ext"
    sh "link /OUT:\"#{TARGET}\" /INCREMENTAL /DLL /DEF:\"ext/#{$name}.def\" /SUBSYSTEM:CONSOLE /DYNAMICBASE:NO /MACHINE:X86 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ext/#{$name}.obj ext/#{$name}.res"
  end
  file "ext/#{$name}.res" => "ext/#{$name}.rc" do |t|
    sh "rc #{t.prerequisites.first}"
  end
when :linux
  file "#{TARGET}" => ["ext/#{$name}.o"] do |t|
      sh "#{CC} #{CFLAGS} #{INCLUDE} -shared #{t.prerequisites.join(' ')} -o #{t.name} -DVERSION=\"#{$version}\""
  end

  file "ext/#{$name}.o" => ["ext/#{$name}.cc", "ext/#{$name}.h"] do |t|
    sh "#{CC} #{CFLAGS} #{INCLUDE} -c #{t.prerequisites.first} -o #{t.name} -DVERSION=\"#{$version}\""
  end
end

# vim: syntax=ruby fileencoding=utf-8
