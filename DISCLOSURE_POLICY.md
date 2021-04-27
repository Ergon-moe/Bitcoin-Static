# Responsible Disclosure Policy

Bitcoin Static takes security very seriously.  We greatly appreciate any and all disclosures of bugs and vulnerabilities that are done in a responsible manner.  We will engage responsible disclosures according to this policy and put forth our best effort to fix disclosed vulnerabilities as well as reaching out to numerous node operators to deploy fixes in a timely manner.

This disclosure policy is also intended to conform to [this proposed standard](https://github.com/RD-Crypto-Spec/Responsible-Disclosure/blob/184391fcbc1bbf3c158c527a841e611ac9ae8388/README.md) with some modifications (see below).

## Responsible Disclosure Guidelines

Do not disclose any security issue or vulnerability on public forums, message boards, mailing lists, etc. prior to responsibly disclosing to Bitcoin Static and giving sufficient time for the issue to be fixed and deployed.
Do not execute on or exploit any vulnerability.  This includes testnet, as both mainnet and testnet exploits are effectively public disclosure.  Regtest mode may be used to test bugs locally.

## Reporting a Bug or Vulnerability

When reporting a bug or vulnerability, please provide the following to security@bitcoincashnode.org:

* A short summary of the potential impact of the issue (if known).
* Details explaining how to reproduce the issue or how an exploit may be formed.
* Your name (optional).  If provided, we will provide credit for disclosure.  Otherwise, you will be treated anonymously and your privacy will be respected.
* Your email or other means of contacting you.
* A PGP key/fingerprint for us to provide encrypted responses to your disclosure.  If this is not provided, we cannot guarantee that you will receive a response prior to a fix being made and deployed.

## Encrypting the Disclosure

We highly encourage all disclosures to be encrypted to prevent interception and exploitation by third-parties prior to a fix being developed and deployed.  Please encrypt using the PGP public key with fingerprint: `5C69 8AA6 589D ED27 EF36  D7FE 9845 1F54 97AB AABF`

It may be obtained via:
```
gpg --recv-keys "5C69 8AA6 589D ED27 EF36  D7FE 9845 1F54 97AB AABF"
```

After this succeeded you can encrypt your prepared disclosure document with the following line:

```
gpg --armor --encrypt --recipient [recipient email address] [yourDisclosureDocument]
```

This will create a copy of your document with the file extension `.asc`, you can email this encrypted version of the document to Bitcoin Statics security team.

If you still have questions about how to do it, feel free to contact the Bitcoin Static project security team for more instructions.

## Backup PGP Keys

These PGP fingerprints and emails are provided only as backups in case you are unable to contact Bitcoin Static via the security email above.

#### Karol Trzeszczkowski
```
Bitcoin Static Lead Maintainer
contact at karol.trzeszczkowski at gmail dot com
5C698AA6589DED27EF36D7FE98451F5497ABAABF
```

## Bounty Payments

Bitcoin Static cannot at this time commit to bounty payments ahead of time.  However, we will use our best judgement and do intend on rewarding those who provide valuable disclosures (with a strong emphasis on easy to read and reproduce disclosures).

## Deviations from the Standard

While Bitcoin Static believes that strong cohesion among neighoring projects and ethical behavior can be standardized to reduce poorly handled disclosure incidents, we also believe that it's in the best interest of Ergon for us to deviate from the standard in the following ways:

* The standard calls for coordinated releases. While Bitcoin Static will make attempts to coordinate releases when possible, it's not always feasible to coordinate urgent fixes for catastrophic exploits (ie. chain splitting events).  For critical fixes, Bitcoin Static will release them in the next release when possible.

## Making changes to this disclosure policy

Note that any changes to this disclosure policy should be mirrored in a pull request to the [Bitcoin Static repository](https://github.com/Ergon-moe/Bitcoin-Static).
