#!/usr/bin/env python3
"""Check retained dependency identities and prevent permission/evidence inflation."""
#20261114-10 add: Verify audit artifacts without executing application/vendor code.
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]


def main():
    """Validate the small policy against real retained files and record every check."""
    inv_path = HERE / 'inventory.json'
    policy_path = ROOT / 'release/stage11-dependencies.json'
    inv = json.loads(inv_path.read_text(encoding='utf-8'))
    policy = json.loads(policy_path.read_text(encoding='utf-8'))
    checks = []
    def check(name, value):
        checks.append({'check': name, 'passed': bool(value)})
    check('policy release restricted', policy['releaseChannel'] == 'restricted_internal_validation' and policy['publicRedistribution'] is False)
    check('policy baseline identity', policy['baselineExecutableSha256'] == inv['executable']['sha256'])
    check('inventory reference sha256', hashlib.sha256(inv_path.read_bytes()).hexdigest() == policy['inventory']['sha256'])
    check('all 23 PE x64', len(inv['peFiles']) == 23 and all(x['architecture'] == 'x86_64' for x in inv['peFiles']))
    check('22 deployed runtime match local source', sum(x.get('matchesInstalledCandidate', False) for x in inv['peFiles']) == 22)
    check('17 Qt SBOM mismatches explicitly retained', sum('qtSbom' in x for x in inv['peFiles']) == 17 and all(x['matchesSbomSha1'] is False for x in inv['peFiles'] if 'qtSbom' in x))
    for license_record in policy['licenseFiles']:
        path = HERE / 'licenses' / Path(license_record['path']).name
        check('license bytes ' + path.name, path.stat().st_size == license_record['bytes'] and hashlib.sha256(path.read_bytes()).hexdigest() == license_record['sha256'])
    check('all 64 source notices retained', len(policy['licenseFiles']) == 64 and sum(x['bytes'] for x in policy['licenseFiles']) == 618734)
    check('no font files copied', not inv['fontFilesInPackage'])
    check('all missing scenarios remain static not run', len(policy['missingDependencyCases']) == 10 and all(x['executionStatus'] == 'not_run_by_static_audit' for x in policy['missingDependencyCases']))
    check('resource references unchanged', all(Path(g['qrc']['path']).exists() and hashlib.sha256(Path(g['qrc']['path']).read_bytes()).hexdigest() == g['qrc']['sha256'] for g in inv['resources']))
    check('all resource bytes unchanged', all(Path(x['path']).stat().st_size == x['bytes'] and hashlib.sha256(Path(x['path']).read_bytes()).hexdigest() == x['sha256'] for g in inv['resources'] for x in g['files']))
    check('independent PE parser crosscheck', json.loads((HERE / 'parser-crosscheck.json').read_text(encoding='utf-8'))['allMatched'])
    check('license whitelist extensions', all(Path(x['path']).suffix in ('.txt', '.html') for x in policy['licenseFiles']))
    for component in policy['components']:
        for source in component.get('sourceFiles', []):
            path = ROOT / source['path']
            check('third-party source ' + source['path'], path.stat().st_size == source['bytes'] and hashlib.sha256(path.read_bytes()).hexdigest() == source['sha256'])
    for path in (ROOT / 'docs/阶段11依赖与许可.md', policy_path, HERE / 'audit_dependencies.py', HERE / 'build_policy.py'):
        check('required output ' + path.name, path.is_file() and path.stat().st_size > 0)
    result = {'format': 'KinSealLite-stage11-dependency-validation-v1', 'checks': checks, 'checkCount': len(checks),
              'allPassed': all(x['passed'] for x in checks), 'policySha256': hashlib.sha256(policy_path.read_bytes()).hexdigest(),
              'componentCount': len(policy['components']), 'noHardwareIo': True, 'noVendorModuleExecution': True,
              'sourceNoticesModified': False}
    (HERE / 'validation.json').write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf-8', newline='\n')
    print(json.dumps({k: v for k, v in result.items() if k != 'checks'}, ensure_ascii=False))
    return 0 if result['allPassed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
