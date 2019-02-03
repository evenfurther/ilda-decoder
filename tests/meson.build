test('check dump',
     files('compare-output.sh'),
     args: [ilda_dump, files('growing_circle.dump'), files('growing_circle.ild')],
     env: ['STRICT_MODE=1'])

test('check liveness 12k',
     files('compare-output.sh'),
     args: [ilda_check_liveness, files('growing_circle.live12k'), files('growing_circle.ild'), '12k'],
     env: ['STRICT_MODE=1'])

test('check liveness 30k',
     files('compare-output.sh'),
     args: [ilda_check_liveness, files('growing_circle.live30k'), files('growing_circle.ild'), '30k'],
     env: ['STRICT_MODE=1'])

test('max records',
     files('compare-output.sh'),
     args: [ilda_max_records, files('growing_circle.records'), files('growing_circle.ild')],
     env: ['STRICT_MODE=1'])
