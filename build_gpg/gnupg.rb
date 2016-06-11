# This is an OS X Homebrew formula. If you have Homebrew installed, you can
# use this to install Homebrew via `brew install path/to/gnupg.rb`.

require "formula"

class Gnupg < Formula
  homepage "http://www.gnupg.org/"
  url "http://mirror.switch.ch/ftp/mirror/gnupg/gnupg/gnupg-1.4.12.tar.bz2"
  sha1 "9b78e20328d35525af7b8a9c1cf081396910e937"
  revision 1

  # bottle do
  #   revision 2
  #   sha1 "e1ea1c3bd682a15370f596a31297eb19ff87998e" => :yosemite
  #   sha1 "71e3618e2f4ea550e194938f6742772fb7d376d9" => :mavericks
  #   sha1 "f933064e91d20ebdb48f6f2180fdf7b99e814b8c" => :mountain_lion
  # end

  option "8192", "Build with support for private keys of up to 8192 bits"

  depends_on "curl" if MacOS.version <= :mavericks

  def install
    inreplace "g10/keygen.c", "max=4096", "max=8192" if build.include? "8192"

    ENV['CC'] = "/usr/local/bin/gcc-4.2"
    ENV['LD'] = "/usr/local/bin/gcc-4.2"
    ENV['CFLAGS'] = "-g -O2"

    system "./configure", "--disable-dependency-tracking",
                          "--prefix=#{prefix}",
                          "--disable-asm"

    system "make"
    system "make check"

    # we need to create these directories because the install target has the
    # dependency order wrong
    [bin, libexec/"gnupg"].each(&:mkpath)
    system "make install"
  end
end
