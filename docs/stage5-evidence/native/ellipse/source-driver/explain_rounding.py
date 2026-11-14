"""Preserve failed pixel evidence and explain the independently verified byte-multiply correction."""
from pathlib import Path
import gzip
import hashlib
import json
import sys

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
WORK=ROOT.parent/'kinseal-stage4-20261112/worktree'
BASE=WORK/'docs/stage5-evidence/native/ellipse'
sys.path.insert(0,str(ROOT/'ellipse_algorithm'))
from area_candidate_256 import coverage


def channels(value):
    """Apply RGB565 truncation to one candidate RGB32 result."""
    return ((value[0]>>3)<<11)|((value[1]>>2)<<5)|(value[2]>>3)


def layer(rgb,alpha,source,old):
    """Compare the old approximation and the observed Qt4 correction on one pixel."""
    def divide(product):
        return (product+128+((product+128)>>8))>>8 if old else (product+(product>>8)+128)>>8
    return [divide(source*alpha)+divide(channel*(255-alpha)) for channel in rgb]


def main():
    """Explain every retained failed pixel without changing or ignoring its native bytes."""
    old=json.loads((BASE/'B-S-M-ellipse-cell-area-r1.json').read_text(encoding='utf-8'))
    new=json.loads((BASE/'B-S-M-ellipse-cell-area-r2.json').read_text(encoding='utf-8'))
    corrected={case['run']:case for case in new['cases']}
    records=[]
    for case in old['cases']:
        if not case['M']['differentPixels']:
            continue
        assert len(case['rects'])==1
        label='geometry-'+'-'.join(map(str,case['rects'][0]))
        masks={name:coverage(gzip.decompress((BASE/'independent-paths-r1'/(label+'-'+name+'-flat.pathstream.gz')).read_bytes())) for name in ('fill','stroke')}
        native=gzip.decompress((BASE/'independent-failures-r1'/(case['run']+'-native.rgb565.gz')).read_bytes())
        previous=gzip.decompress((BASE/'independent-failures-r1'/(case['run']+'-candidate.rgb565.gz')).read_bytes())
        for point in case['M']['firstDifferences']:
            x,y=point['x'],point['y']
            offset=2*(y*800+x)
            assert int.from_bytes(native[offset:offset+2],'little')==point['native']
            assert int.from_bytes(previous[offset:offset+2],'little')==point['candidate']
            fill_alpha=int(masks['fill'][y,x]) if case['fillStyle'] else 0
            stroke_alpha=int(masks['stroke'][y,x]) if case['lineStyle'] else 0
            before=[197,222,255]
            old_fill=layer(before,fill_alpha,255,True)
            new_fill=layer(before,fill_alpha,255,False)
            old_stroke=layer(old_fill,stroke_alpha,0,True)
            new_stroke=layer(new_fill,stroke_alpha,0,False)
            assert channels(old_stroke)==point['candidate'] and channels(new_stroke)==point['native']
            records.append(dict(run=case['run'],x=x,y=y,native565=point['native'],r1Candidate565=point['candidate'],r2Candidate565=channels(new_stroke),fillAlpha=fill_alpha,strokeAlpha=stroke_alpha,backgroundRgb=before,r1AfterWhiteFill=old_fill,r2AfterWhiteFill=new_fill,r1AfterBlackStroke=old_stroke,r2AfterBlackStroke=new_stroke,redDestinationProduct=197*(255-fill_alpha),r1RedDestinationTerm=(197*(255-fill_alpha)+128+((197*(255-fill_alpha)+128)>>8))>>8,r2RedDestinationTerm=(197*(255-fill_alpha)+((197*(255-fill_alpha))>>8)+128)>>8,fullFrameR2DifferentPixels=corrected[case['run']]['M']['differentPixels']))
    public=ROOT/'public-qt-sources/qdrawhelper_p.h'
    result=dict(previousReport='B-S-M-ellipse-cell-area-r1.json',correctedReport='B-S-M-ellipse-cell-area-r2.json',failedPixelsPreserved=True,failedFramesPreserved=True,ignoredPixelMask=[],diagnosticMaskOnlyDoesNotEstablishActualEllipseCoverage=True,fullFrameComparisons=27,allCorrectedFullFramesEqual=new['allRasterComparisonsEqual'],oldFormula='(v+128+((v+128)>>8))>>8',correctFormula='(v+(v>>8)+128)>>8',publicDiagnosticHeader=dict(path=str(public),sha256=hashlib.sha256(public.read_bytes()).hexdigest(),qtDiv255Line=1800,byteMulLine=330),actualNativeState=dict(symbol='QPainter::drawEllipse(const QRectF&)',hints=11,penWidth=0,penStyle='bound per row',penRgba=4278190080,cap=16,join=64,transform=[1,0,0,1,0,0,0,0,1]),nativeBrushRgbaDirectlyObserved=False,whiteFillSemanticsEstablishedByFullFrameComparison=True,pixels=records,nativeProcessLaunched=False,usbAccessed=False)
    (BASE/'rounding-diagnosis-r2.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(dict(failedPixelRecords=len(records),allCorrectedFullFramesEqual=result['allCorrectedFullFramesEqual'],first=records[0]),ensure_ascii=False,indent=2))


if __name__=='__main__':
    main()
