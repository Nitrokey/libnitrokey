import click
import regex as re


@click.command()
@click.argument('input', type=click.File('rt'))
def main(input: click.File):
    print(f"| {'Name':40s} | {'USB ID':9s} |")
    print("|------------------------------------------|:---------:|")
    name = ''
    res = []
    for l in input:
        l = l.strip()
        if l.startswith("#"):
            name = l.strip("#").strip()
            continue
        pid = re.findall(r'ATTR[S]?\{idProduct\}=="(\w+)"', l, re.VERSION1)
        vid = re.findall(r'ATTR[S]?\{idVendor\}=="(\w+)"', l, re.VERSION1)
        if pid:
            res.append((name, vid[0], pid[0]))

    res = sorted(res)
    for r in res:
        if "dev Entry" in r[0]:
            continue
        print(f"| {r[0]:40s} | {r[1]}:{r[2]} |")

    print()
    print(f'Generated from {input.name}')


if __name__ == '__main__':
    main()
