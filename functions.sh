info()
{
    echo "\033[33mInfo:\033[0m $1"
}

error()
{
    echo "\033[31mError:\033[0m $1"
    exit 1
}

check_path()
{
    test -d $1 || error "Path $1 not exists"
}

check_file()
{
    test -f $1 || error "File $1 not exists"
}

check_app()
{
    app=$(which $1 &> /dev/null) || error "$1 not installed"
}

check_required_package()
{
    dpkg --get-selections | grep -q "^$1[[:space:]]*install$" || error "$1 package required"
}

download()
{
    check_app wget
    wget $1 $2 $3
}

unpack()
{
    check_file $1

    filename=$(basename "$1")
    extension="${filename##*.}"

    info "Unpacking $1"
    case ${extension} in
        gz|tgz)
            check_app tar
            tar xvfz $1
        ;;
        xz)
            check_app tar
            tar xJvf $1
        ;;
        bz2)
            check_app tar
            tar xjvf $1
        ;;
        zip)
            check_app unzip
            unzip $1
        ;;
    esac
}

apply_patch()
{
    check_file $1
    info "Applying patch: $1"
    patch -fp1 < $1 || error "Cannot apply patch $1"
}