using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NUnit.Framework;
using ParticleModel;

namespace ParticleEditorTests
{
    [TestFixture]
    public class EmitterSpecTest
    {

        [Test]
        public void TestBrazierRoundtrip()
        {

            var inputLine = @"Brasier	Main Fire	0	Point	perm	100		World		Cartesian	Polar	Sprite	World	Polar	Cartesian	Fire-Sprite	15	Add	0				0			0	0	0	0	0											0	0	0	0	0	0	0		100?300		0		0	0?360	0	-15?15	5?8		1	0?360			0,255,200,150,100,50,0	255(2),255(3),197	64(2),64(3),65	32(2),0(3),0		-23	-95	25	16	20";

            var emitter = EmitterSpec.Parse(inputLine);

            var outputLine = emitter.ToSpec("Brasier");

            var inputArr = inputLine.Split('\t');
            var outputArr = outputLine.Split('\t');

            Assert.AreEqual(inputArr, outputArr);

        }

    }
}
